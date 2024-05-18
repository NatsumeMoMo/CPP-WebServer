#pragma once

#include "Epoll.h"
#include "Channel.h"
// #include "Connection.h"
#include <sys/syscall.h>
#include <functional>
#include <mutex>
#include <queue>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <map>
#include <atomic>

class Connection;

class EventLoop
{
    Epoll *ep_;
    pid_t threadId_;
    std::queue<std::function<void()>> sendQueue; // 存放发送消息的函数,以供IO线程调用
    std::mutex sendQueueMutex;
    int wakeupfd_;          // 用于唤醒IO线程
    Channel *wakeupChannel; // eventfd的Channel
    int timerfd_;
    Channel *timerChannel; // 定时器的Channel
    bool ismainloop_;      // true-是主事件循环，false-是从事件循环

    /* 清除空闲连接 */
    // 1、在事件循环中增加map<int,spConnect> conns_容器，存放运行在该事件循环上全部的Connection对象。
    // 2、如果闹钟时间到了，遍历conns_，判断每个Connection对象是否超时。
    // 3、如果超时了，从conns_中删除Connection对象；
    // 4、还需要从TcpServer.conns_中删除Connection对象。
    std::map<int, std::shared_ptr<Connection>> conns_;
    std::function<void(int)> outtimecallback_;
    std::mutex connsMutex;
    int timetvl_;
    int timeout_;

    std::atomic_bool isStop_;

public:
    EventLoop(bool ismainloop, int timetvl=30, int timeout=90);
    EventLoop(Epoll *ep, bool ismainloop);
    ~EventLoop();
    void runLoop();
    Epoll *getEpoll();

    std::function<void(EventLoop *)> epollouttimecallback_;

    void setEpollOuttimeCallback(std::function<void(EventLoop *)> cb) { epollouttimecallback_ = cb; }

    // 异步唤醒时间循环部分
    bool isInLoopThread() const;             // 获取当前线程是否是EventLoop线程（IO线程）
    void addQueue(std::function<void()> fn); // 把任务添加到队列中
    void wakeup();                           //// 用eventfd唤醒事件循环线程(IO线程)
    void handlewakeup();                     // 事件循环线程被eventfd唤醒后执行的函数

    void handleTimer();  // 闹钟响时执行的函数。
    void addNewConnection(std::shared_ptr<Connection> connect);
    void removeOuttimeConnection(std::shared_ptr<Connection> connect);
    void setOuttimeCallback(std::function<void(int)> cb) { outtimecallback_ = cb; }

    void stop();
};