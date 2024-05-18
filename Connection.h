#pragma once
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include <stdint.h>
#include "InetAddress.h"
#include <functional>
#include "Buffer.h"
#include <sys/syscall.h> //------------>syscall(SYS_gettid);
#include <memory>
#include <atomic>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include <assert.h>
#include <sys/sendfile.h>
#include "Logger.h"

/*
Connection类的主要作用是把服务器与客户端通信fd的Channel封装为一个类。这些功能是原本
是在Channel类中的函数实现

*/

class Connection : public std::enable_shared_from_this<Connection> // 为了使用智能指针版的this
{
    int sep_; // 传给Buffer的消息分割方式

    Buffer rBuf;
    Buffer sBuf;

    EventLoop* Loop_;           // Connection对应的时间循环，由构造函数传入，不属于Connection
    Channel *ConnectionChannel; // 客户端用于通信的Socket，在构造函数中创建
    Sockets *clientSocket_;     // Connection对应的Channel，由构造函数传入，属于Connection
    std::atomic_bool disconnect;  // 客户端连接是否已断开，如果断开则设置为true。 IO线程中改变，工作线程中读取

    std::function<void(std::shared_ptr<Connection>)> closecallback_;
    std::function<void(std::shared_ptr<Connection>)> errorcallback_;
    /* 回调TcpServer::ProcessMessage */
    std::function<void(std::shared_ptr<Connection>, std::string&)> messagecallback_;
    std::function<void(std::shared_ptr<Connection>)> sendcompletecallback_;

    Timestamp lasttime_; // 时间戳，创建Connection对象时为当前时间，每接收到一个报文，把时间戳更新为当前时间

public:
    Connection(EventLoop* loop, Sockets *clientSocket, int sep);
    ~Connection();

    void closeCallback();
    void errorCallback();
    void writeCallback(); // 将发送缓冲区中的内容发送
    void writeFileCallback(std::string filename);

    void setcloseCallback(std::function<void(std::shared_ptr<Connection>)> cb) { closecallback_ = cb; }
    void seterrorCallback(std::function<void(std::shared_ptr<Connection>)> cb) { errorcallback_ = cb; }
    void setmessageCallback(std::function<void(std::shared_ptr<Connection>, std::string&)> cb) { messagecallback_ = cb; }
    void setsendCompleteCallback(std::function<void(std::shared_ptr<Connection>)> cb) { sendcompletecallback_ = cb; }

    int getFd() const;
    std::string getIP() const;
    uint16_t getPort() const;
    Buffer& getrBuf() { return rBuf; }

    // 异步唤醒时间循环部分
    void handleMessage();
    void sendMessage(const char* message, size_t len);
    void sendMessageInLoop(const char *message, size_t len);
    void sendFile(std::string filename);
    void sendFileInLoop(std::string filename);

    bool timeout(time_t now, int val);
};