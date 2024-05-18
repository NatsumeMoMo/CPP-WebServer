#pragma once
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include <stdint.h>
#include "InetAddress.h"
#include "functional"
#include "Connection.h"
/*
Acceptor类的主要作用是把服务器监听fd的Channel封装为一个类。这些功能是原本是在Server类的
构造函数中完成的，现在转移到Acceptor类中，Server类只需要维护一个Acceptor对象，具体监听
的功能由Acceptor类来完成。接受客户端的功能由Channel中的newConnection()函数实现。

*/

class Acceptor
{
    EventLoop *Loop_;         // Acceptor对应的时间循环，由构造函数传入，不属于Acceptor，而属于服务器
                              // 如果要使用智能指针的话就应该设置为 const std::unique_ptr<EventLoop> Loop_
                              // 因为没有所有权所以不能使用移动语义
    Channel *acceptorChannel; // 服务端用于监听的Socket，在构造函数中创建。一个网络服务程序只有一个Acceptor
                              // 所以该Acceptor使用栈内存也可以
    Sockets *ServerSocket;    // Acceptor对应的Channel，在构造函数中创建
    InetAddress *Server_addr;
    std::function<void(Sockets *)> connectCallback_;

public:
    Acceptor(EventLoop *loop, const char *IP, uint16_t Port);
    ~Acceptor();
    void newConnection();

    void setConnectionCallback(std::function<void(Sockets *)> cb);
};