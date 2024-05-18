#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <fcntl.h>
#include "InetAddress.h"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include "Logger.h"

/*
对Socket相关的操作进行封装。除了封装的功能外，其成员函数也对外开放使用
*/

class Sockets
{
private:
    int fd_;         // 不必设为 const，将其设为 private 类型，对外只提供只读接口即可
    std::string IP_; // 如果是listenfd，存放服务器IP，如果是客户端连接的Fd，存放客户端IP
    uint16_t Port_;   // 如果是listenfd，存放服务器监听port，如果是客户端连接的Fd，存放客户端port

public:
    Sockets();
    Sockets(int fd);
    ~Sockets();

    int createTcpSock();
    int bind(const InetAddress &addr);
    int listen(int backlog);
    int accept(InetAddress &addr);

    void setIPandPort(const char* IP, uint16_t Port);
    int getfd() const;
    std::string getip() const;
    uint16_t getport() const;
};

void setUnblock(int sockfd);