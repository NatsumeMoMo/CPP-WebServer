//
// Created by jason on 24-4-27.
//

#ifndef REACTORSERVER_TCPSERVER_H
#define REACTORSERVER_TCPSERVER_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <algorithm>
#include <unistd.h>
#include <memory>
#include <map>
#include <sys/syscall.h>
#include <mutex>

#include "Socket.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "Epoll.h"

class Server
{
public:
    Server(const char *IP, uint16_t Port,int Threadnum = 10);
    ~Server();
    void serverLoop();
    void stop();
    // 改以下Connection为智能指针
    /* 回调历程：EventLoop::runLoop()->Channel::handleEvent()->Acceptor::newConnection()
        ->Server::newConnection() */
    void newConnection(Sockets * clientSocket);
    /* 初始回调过程为 Channel::handleMessage->Connection::closeCallback()->Server::closeConnection() */
    /* 修改后回调过程为 Connection::handleMessage->Connection::closeCallback()->Server::closeConnection() */
    void closeConnection(std::shared_ptr<Connection> connect);
    /* 回调过程应该为 Channel::handleMessage->Connection::errorCallback()->Server::closeConnection()
    但实际的handleMessage中没有实现错误的情况 */
    void errorConnection(std::shared_ptr<Connection> connect);
    /* 回调历程：Channel::handleEvent()->Connection::handleMessage()->Server::ProcessMessage() */
    void ProcessMessage(std::shared_ptr<Connection> connect, std::string &message);
    /* 回调过程为 Connection::setsendCompleteCallback()->Server::sendMessageNotify() */
    void sendMessageNotify(std::shared_ptr<Connection> connect);
    /* 回调过程为 EventLoop::runLoop()->Server::epollTimeout() */
    void epollTimeout(EventLoop *loop);

    void removeConnection(int fd); // 删除超时的连接

    void setEchoNewConnectionCallback(std::function<void(std::shared_ptr<Connection>)> cb) { EchonewConnectioncallback_ = cb; }
    void setEchoCloseCallback(std::function<void(std::shared_ptr<Connection>)> cb) { Echoclosecallback_ = cb; }
    void setEchoErrorCallback(std::function<void(std::shared_ptr<Connection>)> cb) { Echoerrorcallback_ = cb; }
    void setEchoMessageCallback(std::function<void(std::shared_ptr<Connection>, std::string &)> cb) { Echomessagecallback_ = cb; }
    void setEchoSendCompleteCallback(std::function<void(std::shared_ptr<Connection>)> cb) { Echosendcompletecallback_ = cb; }
    void setEchoTimeoutCallback(std::function<void(EventLoop*)> cb) { Echotimeoutcallback_ = cb; }


private:
    EventLoop *ServerLoop_; 
    std::vector<EventLoop *> subloops_;      // 子循环l
    Acceptor *acceptor_;                     // 一个TcpServer只有一个Acceptor对象。
    std::map<int, std::shared_ptr<Connection>> connections; // 替代了原本的ClientInfos的功能
    ThreadPool *pool;
    std::mutex mutex_;

    /* TcpServer 的连接分配策略:
        采用轮询（Round Robin）和动态分配相结合的策略, 使Connection更加均匀地分配到各个子线程（subEventLoop），
        并且根据线程池中新创建的工作线程动态分配 subEventLoop
        
        1、在 TcpServer 类中添加一个轮询的计数器，用于记录下一个连接将分配到哪个 subEventLoop
        
        2、在 newConnection 方法中，使用轮询计数器来分配连接
        
        3、在 ThreadPool 的MangerFunction()中, 添加新线程时为其分配一个 subEventLoop */

    /* 轮询的计数器 */
    std::atomic<int> nextSubLoopIndex_;
    int MaxSubLoopNum_;

    // 改以下Connection为智能指针
    std::function<void(std::shared_ptr<Connection>)> EchonewConnectioncallback_;
    std::function<void(std::shared_ptr<Connection>)> Echoclosecallback_;
    std::function<void(std::shared_ptr<Connection>)> Echoerrorcallback_;
    std::function<void(std::shared_ptr<Connection>, std::string &)> Echomessagecallback_;
    std::function<void(std::shared_ptr<Connection>)> Echosendcompletecallback_;
    std::function<void(EventLoop* )> Echotimeoutcallback_;
};

#endif // REACTORSERVER_TCPSERVER_H
