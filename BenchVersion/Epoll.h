#pragma once
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include "Logger.h"

class Channel;

class Epoll
{
private:
    static const int EventsSize = 1024;
    int epfd_;
    epoll_event ev_;
    epoll_event evs_[EventsSize];
public:
    Epoll();
    ~Epoll();

    int addEpollFd(int fd, int opt);
    void updateChannel(Channel *ch); // 把channnel添加\更新到红黑树上，Channel中有fd，也有需要监视的事件
    void removeChannel(Channel *ch); // 把channel从红黑树上删除
    int delEpollFd(int fd);

    std::vector<Channel*> EpollLoop(int timeout=-1);
};