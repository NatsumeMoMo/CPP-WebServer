#include "EventLoop.h"
#include "Connection.h"
int createtimerfd(int sec=90)
{
    int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK);   // 创建timerfd
    struct itimerspec timeout;                                // 定时时间的数据结构
    memset(&timeout,0,sizeof(struct itimerspec));
    timeout.it_value.tv_sec = sec;                             // 定时时间
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd,0,&timeout,0);
    return tfd;
}

EventLoop::EventLoop(bool ismainloop,int timetvl, int timeout) : ep_(new Epoll()), ismainloop_(ismainloop)
{
    wakeupfd_ = eventfd(0, EFD_NONBLOCK);
    wakeupChannel = new Channel(ep_, wakeupfd_);
    wakeupChannel->setReadCallback(std::bind(&EventLoop::handlewakeup, this));
    wakeupChannel->enableReading();

    timetvl_ = timetvl;
    timeout_ = timeout;
    timerfd_ = createtimerfd(timeout_);
    timerChannel = new Channel(ep_, timerfd_);
    timerChannel->setReadCallback(std::bind(&EventLoop::handleTimer, this));
    timerChannel->enableReading();

    isStop_ = false;
}

EventLoop::EventLoop(Epoll* ep, bool ismainloop) : ep_(ep), ismainloop_(ismainloop)
{
    wakeupfd_ = eventfd(0, EFD_NONBLOCK);
    wakeupChannel = new Channel(ep_, wakeupfd_);
    wakeupChannel->setReadCallback(std::bind(&EventLoop::handlewakeup, this));
    wakeupChannel->enableReading();

    timerfd_ = createtimerfd();
    timerChannel = new Channel(ep_, timerfd_);
    timerChannel->setReadCallback(std::bind(&EventLoop::handleTimer, this));
    timerChannel->enableReading();

    isStop_ = false;
}

EventLoop::~EventLoop()
{
    delete wakeupChannel;
    delete timerChannel;
}

void EventLoop::runLoop()
{
    // 主循环和子循环均会执行该函数，且Server的线程池中添加到任务队列的都是该函数，因此线程id在该处获取
    threadId_ = syscall(SYS_gettid); 
    while (isStop_ == false)
    {
        auto channels = ep_->EpollLoop(); // 可传参 比如5*1000

        if(channels.size()==0)
            /* 回调Server::epollTimeout(EventLoop* loop) */
            epollouttimecallback_(this);
        for (auto &ch : channels)
        {
            ch->handleEvent();
        }
    }
}

Epoll* EventLoop::getEpoll()
{
    return ep_;
}

bool EventLoop::isInLoopThread() const
{
    return threadId_ == syscall(SYS_gettid);
}

void EventLoop::addQueue(std::function<void()> fn)
{
    {
        std::lock_guard<std::mutex> lock(sendQueueMutex);
        sendQueue.push(fn);
    }
    wakeup();
}

void EventLoop::wakeup()
{
    uint64_t val=1;
    write(wakeupfd_,&val,sizeof(val));
}

void EventLoop::handlewakeup()
{
    uint64_t val;
    // 从eventfd中读取出数据，如果不读取水平模式下eventfd的读事件会一直触发。
    read(wakeupfd_,&val,sizeof(val));       

    std::function<void()> fn;

    std::lock_guard<std::mutex> lock(sendQueueMutex);
    while (!sendQueue.empty())
    {
        fn=std::move(sendQueue.front());    // 出队一个元素。
        sendQueue.pop();                              
        fn();  
   }
}

void EventLoop::handleTimer()
{
    struct itimerspec timeout;
    memset(&timeout, 0, sizeof(timeout));
    timeout.it_value.tv_sec = timetvl_;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_, 0, &timeout, 0);

    if(ismainloop_)
        // printf("主事件循环时间到\n");
        LOG_INFO("主事件循环时间到");
    else
    {
        printf("EventLoop::handleTimer() thread id is %ld. Fd: \n",syscall(SYS_gettid));
        time_t now=time(0);         // 获取当前时间。

        for(auto it = conns_.begin(); it != conns_.end(); ++it)
        {
            printf("%d ",it->first);
            if(it->second->timeout(now,timeout_))
            {
                removeOuttimeConnection(it->second);
                {
                    std::lock_guard<std::mutex> lock(connsMutex);
                    it = conns_.erase(it); // 使用返回的迭代器更新it
                }

            }

        }
    }
}

void EventLoop::addNewConnection(std::shared_ptr<Connection> connect)
{
    std::lock_guard<std::mutex> lock(connsMutex);
    conns_[connect->getFd()] = connect;
}

void EventLoop::removeOuttimeConnection(std::shared_ptr<Connection> connect)
{
    outtimecallback_(connect->getFd());
}

void EventLoop::stop()
{
    isStop_ = true;
    wakeup();  // 唤醒事件循环，如果没有这行代码，事件循环将在下次闹钟响时或epoll_wait()超时时才会停下来。
}