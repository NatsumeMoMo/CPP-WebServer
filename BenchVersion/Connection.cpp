#include "Connection.h"
#include <thread>
#include <chrono>

Connection::Connection(EventLoop *loop, Sockets *clientSocket) : Loop_(loop), clientSocket_(clientSocket),
                                                                 disconnect(false)
{
    ConnectionChannel = new Channel(Loop_->getEpoll(), clientSocket->getfd());
    ConnectionChannel->setReadCallback(std::bind(&Connection::handleMessage, this));
    ConnectionChannel->setCloseCallback(std::bind(&Connection::closeCallback, this));
    ConnectionChannel->setErrorCallback(std::bind(&Connection::errorCallback, this));
    ConnectionChannel->setWriteCallback(std::bind(&Connection::writeCallback, this));
    ConnectionChannel->use_ET();
    ConnectionChannel->enableReading();
}

Connection::~Connection()
{
    delete clientSocket_; // 外部接受客户端创建Socket时不知道怎样处理，将其交给Connection对象
                          // 客户端的Socket应该与Connection的生存周期一致
    delete ConnectionChannel;
}


int Connection::getFd() const
{
    return clientSocket_->getfd();
}

std::string Connection::getIP() const
{
    return clientSocket_->getip();
}

uint16_t Connection::getPort() const
{
    return clientSocket_->getport();
}

void Connection::closeCallback()
{
    disconnect = true;
    ConnectionChannel->disableAll();

    /*
        最开始想的是closeCallback()中会对epoll中的fd进行删除。但后面觉得应该还是每一个EventLoop
        一个Epoll较为合适，而不是整个Server只使用一个Epoll，最开始ServerLoop中的Epoll还是只需要
        负责接受新的客户端即可，后续对每个客户端的处理还是由每个EventLoop中的Epoll负责即可
     */
    Loop_->getEpoll()->removeChannel(ConnectionChannel); // closeCallback()中会对epoll中的fd进行删除
    closecallback_(shared_from_this());                  // 使用智能指针的this
}
void Connection::errorCallback()
{
    disconnect = true;
    ConnectionChannel->disableAll();
    Loop_->getEpoll()->removeChannel(ConnectionChannel);
    errorcallback_(shared_from_this());
}

void Connection::handleMessage()
{
    char buffer[1024];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int recvlen = recv(getFd(), buffer, sizeof(buffer), 0);

        if (recvlen > 0)
        {
            rBuf.append(buffer, recvlen);

            std::string message;
            while (rBuf.pickmessage(message))
            {
                // LOG_INFO("Thread %ld recv %d client's message: %s.", syscall(SYS_gettid), getFd(), message.c_str());

                // LOG_INFO("Thread %ld recv %d client's message.", syscall(SYS_gettid), getFd());
                messagecallback_(shared_from_this(), message);
            }
        }
        else if (recvlen == 0)
        {
            // Client closed the connection
            closeCallback();
            break;
        }
        else
        {
            if (errno != EAGAIN)
            {
                // std::cerr << "Recv error: " << strerror(errno) << std::endl;
                LOG_ERROR("Recv error: %s", strerror(errno));
                closeCallback();
            }

            if (errno == EINTR)
                continue;

            break;
        }
    }
}

void Connection::sendMessage(const char *message, size_t len)
{
    if (disconnect == true)
    {;
        LOG_INFO("Send returned because disconnect");
        return;
    }
    if (Loop_->isInLoopThread())
    {
        // 如果当前线程是IO线程，直接调用sendinloop()发送数据。
        sendMessageInLoop(message, len);
    }
    else
    {
        // 如果当前线程不是IO线程，调用EventLoop::queueinloop()，把sendinloop()交给事件循环线程去执行。
        // Connection层次无法确保自己是被IO线程执行还是被工作线程执行，因此需要把发送的数据的部分回调到EventLoop层次
        Loop_->addQueue(std::bind(&Connection::sendMessageInLoop, shared_from_this(), message, len));
    }
}

void Connection::sendMessageInLoop(const char *message, size_t len)
{
    sBuf.appendwithsep(message, len); // 把需要发送的数据保存到Connection的发送缓冲区中。
    /* HTTP 服务器不能采用注册写事件的方式, 而是需要直接发送 */
    writeCallback();
}

void Connection::writeCallback()
{
    int write_len = send(getFd(), sBuf.data(), sBuf.size(), 0);
//    LOG_INFO("Thread %ld send message out.", syscall(SYS_gettid));
    if (write_len > 0) {
        sBuf.erase(0, write_len);
    }
    if (sBuf.size() == 0) {
        ConnectionChannel->disableWriting();
    } else {
        ConnectionChannel->enableWriting();
    }
    sendcompletecallback_(shared_from_this());
}

bool Connection::timeout(time_t now, int val)
{
    return now - lasttime_.toint() > val;
}
