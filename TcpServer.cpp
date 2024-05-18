//
// Created by jason on 24-4-27.
//

#include "TcpServer.h"

// 默认值仅在TcpServer.h中给出即可，在TcpServer.cpp中需要移除这些默认值
Server::Server(const char *IP, uint16_t Port, int sep, int Threadnum)
    : nextSubLoopIndex_(0), SubLoopNum_(Threadnum), sep_(sep)
{
    ServerLoop_ = new EventLoop(true);
    acceptor_ = new Acceptor(ServerLoop_, IP, Port);
    pool = new ThreadPool(Threadnum);
    acceptor_->setConnectionCallback(std::bind(&Server::newConnection, this, std::placeholders::_1));
    ServerLoop_->setEpollOuttimeCallback(std::bind(&Server::epollTimeout, this, std::placeholders::_1));
    for (int i = 0; i < Threadnum; i++)
    {
        subloops_.push_back(new EventLoop(false));
        subloops_[i]->setEpollOuttimeCallback(std::bind(&Server::epollTimeout, this, std::placeholders::_1));
        subloops_[i]->setOuttimeCallback(std::bind(&Server::removeConnection, this, std::placeholders::_1));
        pool->enqueue(std::bind(&EventLoop::runLoop, subloops_[i]));
    }
}

Server::~Server()
{
    // 线程池一定要最先释放，否则会造成其他资源释放后线程池执行释放工作时访问其他已释放的资源而引起段错误
    delete pool;  
    delete ServerLoop_;
    delete acceptor_;
    for (auto &lp : subloops_)
    {
        delete lp;
    }
    printf("IO线程池停止。\n");
}

void Server::newConnection(Sockets *clientSocket)
{
    int index = nextSubLoopIndex_++ % SubLoopNum_;
    std::shared_ptr<Connection> clientConnection = std::make_shared<Connection>(subloops_[index], clientSocket, sep_);
    clientConnection->setcloseCallback(std::bind(&Server::closeConnection, this, std::placeholders::_1));
    clientConnection->seterrorCallback(std::bind(&Server::errorConnection, this, std::placeholders::_1));
    clientConnection->setmessageCallback(std::bind(&Server::ProcessMessage, this, std::placeholders::_1, std::placeholders::_2));
    clientConnection->setsendCompleteCallback(std::bind(&Server::sendMessageNotify, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connections.insert(std::make_pair(clientSocket->getfd(), clientConnection));
    }
    // 把当前的connection 加入到对应的subloop中，以判断该connection是否超时
    subloops_[index]->addNewConnection(clientConnection);

    LOG_INFO("Thread %ld accept a client. It's IP is %s ,and its port is %d. ",
        syscall(SYS_gettid), clientConnection->getIP().c_str(), clientConnection->getPort());
    // 业务需求，在以下回调函数中进行处理

    /*  TcpServer只是为上层业务类提供了可能会用到的接口，但是否被上层业务类使用要根据实际
        的开发情况决定。所以处于底层的TcpServer类执行上层业务类的回调时要先判断该回调函数
        是否存在，若存在再执行 */

    /*  新建连接的业务处理要在连接正式建立后再执行 */
    if (EchonewConnectioncallback_)
        EchonewConnectioncallback_(clientConnection);
}

void Server::serverLoop()
{
    ServerLoop_->runLoop();
}

void Server::stop()
{
    ServerLoop_->stop();
    LOG_INFO("主事件循环已停止");

    for(auto &it : subloops_)
        it->stop();
    LOG_INFO("从事件循环已停止");
}


void Server::closeConnection(std::shared_ptr<Connection> connect)
{
    std::unique_lock<std::mutex> lock(mutex_);
    // 业务需求，在以下回调函数中进行处理
    /*  断开连接的业务处理要在连接断开前执行 */
    if (Echoclosecallback_)
        Echoclosecallback_(connect);
    connections.erase(connect->getFd());
    LOG_INFO("Fd : %d Client closed the connection.", connect->getFd());
}


void Server::errorConnection(std::shared_ptr<Connection> connect)
{
    // 业务需求，在以下回调函数中进行处理
    if (Echoerrorcallback_)
        Echoclosecallback_(connect);
    close(connect->getFd());
}

void Server::ProcessMessage(std::shared_ptr<Connection> connect, std::string &message)
{
    // 业务需求，在以下回调函数中进行处理
    if (Echomessagecallback_)
        Echomessagecallback_(connect, message);
}

void Server::sendMessageNotify(std::shared_ptr<Connection> connect)
{
    // 根据业务的需求，在以下回调函数中进行处理
    if (Echosendcompletecallback_)
        Echosendcompletecallback_(connect);
}

void Server::epollTimeout(EventLoop *loop)
{
    // 根据业务的需求，在以下回调函数中进行处理

    if (Echotimeoutcallback_)
        Echotimeoutcallback_(loop);
}

void Server::removeConnection(int fd)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connections.erase(fd);
}