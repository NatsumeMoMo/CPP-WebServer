#pragma once
#include <sys/epoll.h>
#include <memory>
#include <map>
#include <functional>
#include "Socket.h"
#include "InetAddress.h"

class Epoll;

class ClientInfo
{
public:
    int clientSocketfd;
    bool connectionClosed;
    std::unique_ptr<InetAddress> client_addr_;
    ClientInfo(int clientfd, sockaddr_in caddr) : clientSocketfd(clientfd), connectionClosed(false)
    {
        client_addr_ = std::make_unique<InetAddress>(caddr);
    }
};

class Channel
{
public:
    using EventCallback = std::function<void()>;

private:
    int fd_ = -1;               // Channel和fd是一对一的关系
    Epoll* ep_; /*Channel对应的红黑树，Channel和Epoll是多对一的关系，一个Channel只
                                           对应一个Epoll*/

    bool isInepoll = false; /* Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()时用 EPOLL_CTL_ADD ，否则
                            调用EPOLL_CTL_MOD*/

    int events_ = 0;     // fd_需要监听的事件。listenfd和clientfd需要读事件EPOLLIN，client还可能需要监视些事件EPOLLOUT
    int revents_ = 0;    // fd_已发生的事件。在最近一次事件轮询中，该`Channel`上实际发生的事件类型。
    int lastevents_ = 0; // 上一次事件
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_; // 回调Connection类的closeConnection()
    EventCallback errorCallback_; // 回调Connection类的errorConnection()
    EventCallback writeFileCallback_;
    static std::map<int, std::unique_ptr<ClientInfo>> clientInfos; // 存储客户端信息

public:
    Channel(Epoll* ep, int fd) : ep_(ep), fd_(fd) {}
    ~Channel();
    int getChannelFd() const { return fd_; }
    void use_ET();        // 采用边缘触发模式
    void enableReading(); // 让 epoll_wait() 监视fd_的读事件
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();   // 屏蔽所有事件
    void setInEpoll();                     // 把 isInepoll 成员设置为true
    bool isInEpoll() { return isInepoll; } // 返回 isInepoll 成员
    int getEvents() const { return events_; }
    void setRevents(uint32_t revents) { revents_ = revents; } // 设置 revents_ 成员的值
    int getRevents() const { return revents_; }

    void handleEvent();   // 事件处理函数，epoll_wait() 返回的时候执行
    void handleMessage(); // 处理客户端发送的消息

    void setReadCallback(const EventCallback &cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback &cb) { writeCallback_ = cb; }
    void setCloseCallback(const EventCallback &cb) { closeCallback_ = cb; }
    void setErrorCallback(const EventCallback &cb) { errorCallback_ = cb; }
    void setWriteFileCallback(const EventCallback &cb) { writeFileCallback_ = cb; }
};