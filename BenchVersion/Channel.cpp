#include "Channel.h"
#include "Epoll.h"

std::map<int, std::unique_ptr<ClientInfo>> Channel::clientInfos;

Channel::~Channel()
{
    // 析构函数什么也不做。不能销毁ep、关闭fd。因为这两个东西不属于Channel，只是需要使用他们而已
    // 因为创建Channel对象时这两个值是从类外传入的
}

void Channel::enableReading()
{
    events_ = events_ | EPOLLIN;
    ep_->updateChannel(this);
}

void Channel::disableReading()
{
    events_ &= ~EPOLLIN;
    ep_->updateChannel(this);
}

void Channel::setInEpoll()
{
    isInepoll = true;
}

void Channel::use_ET()
{
    events_ = events_ | EPOLLET;
}

void Channel::enableWriting()
{
    events_ = events_ | EPOLLOUT;
    ep_->updateChannel(this);
}

void Channel::disableWriting()
{
//    events_ = events_ | ~EPOLLOUT;
    events_&=~EPOLLOUT;
    ep_->updateChannel(this);
}

void Channel::disableAll()
{
    events_ = 0;
    ep_->updateChannel(this);
}


void Channel::handleEvent()
{
    // readCallback_();

    /* 这里使用的是 revents_ 而不是 events_ . 因为events_是要监听的事件，是事先设置的，而revents_是EPOLL最近
     *一次时间轮询中，该Channel上实际发生的事件类型。要根据实际的事件执行相应的回调函数
     * */

    if(revents_ & EPOLLRDHUP)  // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0
    {
        closeCallback_();
    }
    else if(revents_ & (EPOLLIN | EPOLLPRI)) // 接受缓冲区中有数据可读
    {
        // 回调 Acceptor 类中的newConnection
        // 客户端通信fd则回调Connection中的handleMessage()
        readCallback_();
    }
    else if(revents_ & EPOLLOUT)
    {
        // 回调 Connection 类中的writeCallback()
        writeCallback_();
    }
    else
    {
        errorCallback_();
    }
}