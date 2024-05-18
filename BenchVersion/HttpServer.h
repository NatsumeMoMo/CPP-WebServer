//
// Created by jason on 24-4-8.
//

#pragma once

#include "TcpServer.h"
#include <string>
#include <stdint.h>
#include "ThreadPool.h"
#include <memory>
#include "Buffer.h"

#include <sys/stat.h>
#include <cstring>
#include <strings.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <functional>

/*
之前一直是TcpServer充当业务处理的功能，但实际上TcpServer和其他一样，也是属于底层类。
因此对于业务的处理部分应该由回显服务来完成
*/
class HttpServer
{
private:
    std::string IP_;
    uint16_t Port_;
    Server *TcpServer;

    // HttpRequest* request;

//    Buffer buf;

public:
    HttpServer(const std::string IP, const uint16_t Port);
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




    int parseRequestLine(const char *line, std::shared_ptr<Connection> connect);
    int sendFile(const char* fileName, int cfd);
    int sendDir (std::string filename, std::shared_ptr<Connection> connect);
    const char* getFileType(const char* name);
    int hexToDec(char c);
    void decodeMsg(char* to, char* from);
    // void responseHeadMsg(int status, const char* descr, const char* type, int length, std::string &buf);
    void responseHeadMsg(int status, const char *descr,
                                 const char *type, int length, std::shared_ptr<Connection> connect);

    void sendHelloWorld(std::shared_ptr<Connection> connect);

};

