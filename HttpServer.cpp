#include "HttpServer.h"

HttpServer::HttpServer(const std::string IP, const uint16_t Port, int sep) : TcpServer(new Server(IP.c_str(), Port, sep))
{
    TcpServer->setEchoNewConnectionCallback(std::bind(&HttpServer::HandleNewConnection, this, std::placeholders::_1));
    TcpServer->setEchoCloseCallback(std::bind(&HttpServer::HandleCloseConnection, this, std::placeholders::_1));
    TcpServer->setEchoErrorCallback(std::bind(&HttpServer::HandleErrorConnection, this, std::placeholders::_1));
    TcpServer->setEchoMessageCallback(std::bind(&HttpServer::HandleMessage, this, std::placeholders::_1,
                                                std::placeholders::_2));
    TcpServer->setEchoSendCompleteCallback(std::bind(&HttpServer::HandleSendMessageNotify, this,
                                                     std::placeholders::_1));
    TcpServer->setEchoTimeoutCallback(std::bind(&HttpServer::HandleTimeout, this, std::placeholders::_1));

    response_ = new HttpResponse();
    response_->setOnMessageCallbakc(std::bind(&HttpServer::OnMessage,this, std::placeholders::_1, std::placeholders::_2));
    request_ = new HttpRequest(response_);
    request_->setOnMessageCallbakc(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1,
                                            std::placeholders::_2));

    // pool = new ThreadPool(10); // 工作线程池
}

HttpServer::~HttpServer()
{
    delete TcpServer;
    LOG_INFO("TcpServer已停止");

    delete request_;
    delete response_;
}

void HttpServer::Start()
{
    TcpServer->serverLoop();
}

void HttpServer::Stop()
{
    TcpServer->stop();
}

// 处理新客户端连接请求后的业务需求，在TcpServer类中回调此函数。
void HttpServer::HandleNewConnection(std::shared_ptr<Connection> connect)
{

    // 根据业务的需求，在这里可以增加其它的代码。
    // printf("%s client starts!\n", Timestamp::now().tostring().c_str());
}

// 关闭新客户端连接请求后的业务需求，在TcpServer类中回调此函数。
void HttpServer::HandleCloseConnection(std::shared_ptr<Connection> connect)
{
    // 根据业务的需求，在这里可以增加其它的代码。
}

void HttpServer::HandleErrorConnection(std::shared_ptr<Connection> connect)
{
    // 根据业务的需求，在这里可以增加其它的代码。
    LOG_INFO("client which fd is %d connect error", connect->getFd());
}

// 客户端的连接错误后的业务处理，在TcpServer类中回调此函数。
void HttpServer::HandleMessage(std::shared_ptr<Connection> connect, std::string &message)
{

    bool flag = request_->parseHttpRequest(message, connect);
}

// 处理客户端的请求报文，用于添加给线程池
void HttpServer::OnMessage(std::shared_ptr<Connection> connect,const std::string &message)
{
    connect->sendMessageInLoop(message.data(), message.size()); // 发送缓冲区中的数据

}

// 数据发送完成后的业务处理，在TcpServer类中回调此函数。
void HttpServer::HandleSendMessageNotify(std::shared_ptr<Connection> connect)
{
    // 根据业务的需求，在这里可以增加其它的代码。
}

// epoll_wait超时后的业务处理，在TcpServer类中回调此函数。
void HttpServer::HandleTimeout(EventLoop *loop)
{
    // 根据业务的需求，在这里可以增加其它的代码。
    LOG_INFO("epoll_wait() timeout.");
}