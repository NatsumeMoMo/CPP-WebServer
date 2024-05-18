#include "Epoll.h"
#include "Channel.h"

Epoll::Epoll()
{
    epfd_ = epoll_create(1);
}

Epoll::~Epoll()
{
    close(epfd_);
}

int Epoll::addEpollFd(int fd, int opt) // 多余
{
    ev_.events = opt;
    ev_.data.fd = fd;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev_) == -1)
    {
        std::cout << "epoll_ctl add client error!" << std::endl;
        return -1;
    }
    return 0;
}

void Epoll::updateChannel(Channel *ch)
{
    epoll_event ev;
    ev.data.ptr = ch;             // 指定Channel
    ev.events = ch->getEvents(); // 指定事件
    if (ch->isInEpoll())          // 如果Channel已经在红黑树上，则进行更新
    {
        if(epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->getChannelFd(), &ev) == -1)
        {
            // perror("epoll_ctl() MOD failed\n");
            LOG_ERROR("epoll_ctl() MOD failed: %s", strerror(errno));
            exit(-1);
        }
    }
    else // 如果不在红黑树上则进行添加
    {
        if(epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->getChannelFd(), &ev) == -1)
        {
            // perror("epoll_ctl() ADD failed\n");
            LOG_ERROR("epoll_ctl() ADD failed: %s", strerror(errno));

            exit(-1);
        }
        ch->setInEpoll();
    }
}

void Epoll::removeChannel(Channel *ch)
{
    if(ch->isInEpoll())
    {
        if (epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->getChannelFd(), 0) == -1)
        {
            // std::cout << "epoll_ctl del client error!" << std::endl;
            LOG_INFO("epoll_ctl del client error!");
        }
    }

}

int Epoll::delEpollFd(int fd)
{
    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        return -1;
    }
    else
    {
        // std::cout << "Success to remove "<< fd << " from epoll! " << std::endl;
        LOG_INFO("Success to remove %d from epoll!", fd);
    }
    return 0;
}

std::vector<Channel*> Epoll::EpollLoop(int timeout)
{
    int retnum = epoll_wait(epfd_, evs_, EventsSize, timeout);
    std::vector<Channel*> channels; // 使用局部变量，并直接返回
    if (retnum < 0)
    {
        // EBADF ：epfd不是一个有效的描述符。
        // EFAULT ：参数events指向的内存区域不可写。
        // EINVAL ：epfd不是一个epoll文件描述符，或者参数maxevents小于等于0。
        // EINTR ：阻塞过程中被信号中断，epoll_pwait()可以避免，或者错误处理中，解析error后重新调用epoll_wait()。
        // 在Reactor模型中，不建议使用信号，因为信号处理起来很麻烦，没有必要。------ 陈硕
        // perror("Epoll_wait() failed");
        LOG_ERROR("Epoll_wait() failed: %s", strerror(errno));
        exit(-1);
    }
    else if (retnum == 0)
    {
        // 如果epoll_wait()超时，表示系统很空闲，返回的channels将为空。
        return channels;
    }
    else
    {
        for (int i = 0; i < retnum; i++)
        {
            Channel* ch = (Channel*)evs_[i].data.ptr; // 取出已发生事件的Channel
            ch->setRevents(evs_[i].events); // 设置Channel的revents_成员
            channels.push_back(ch);
        }
    }
    return channels;
}
