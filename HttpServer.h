#pragma once

#include "TcpServer.h"
#include <string>
#include <stdint.h>
#include <memory>
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpServer
{
private:
    std::string IP_;
    uint16_t Port_;
    Server *TcpServer;

    /*  工作线程池, Server中的是IO线程池, 但本WebServer中未使用该线程池, 所有的工作
        都是由Server中的IO线程池完成。因为经测试后如果是工作线程处理HTTP逻辑，把与浏
        览器通信的工作传回给IO线程的话, 浏览器会介绍不到数据。具体原因未知
     */
    ThreadPool* pool;

    HttpRequest* request_;
    HttpResponse* response_;


public:
    HttpServer(const std::string IP, const uint16_t Port, int sep);
    ~HttpServer();

    /* 回调历程：EventLoop::runLoop()->Channel::handleEvent()->Acceptor::newConnection()
        ->Server::newConnection()->EchoServer::HandleNewConnection() */
    void HandleNewConnection(std::shared_ptr<Connection> connect);
    /* 修改后回调过程为 Connection::handleMessage->Connection::closeCallback()->Server::closeConnection()
        ->EchoServer::HandleCloseConnection() */
    void HandleCloseConnection(std::shared_ptr<Connection> connect);
    /* 回调过程为 Channel::handleMessage->Connection::errorCallback()->Server::closeConnection()
        ->EchoServer::HandleErrorConnection() */
    void HandleErrorConnection(std::shared_ptr<Connection> connect);

    void HandleMessage(std::shared_ptr<Connection> connect, std::string &message);
    /* 回调过程为 Connection::setsendCompleteCallback()->Server::sendMessageNotify()
        ->EchoServer::HandleSendMessageNotify */
    void HandleSendMessageNotify(std::shared_ptr<Connection> connect);
    /* 回调过程为 EventLoop::runLoop()->Server::epollTimeout()->EchoServer::HandleTimeout */
    void HandleTimeout(EventLoop *loop);

    void OnMessage(std::shared_ptr<Connection> connect ,const std::string &message);

    void Start();

    void Stop();
};

