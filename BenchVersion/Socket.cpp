#include "Socket.h"

Sockets::Sockets()
{
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
}

Sockets::Sockets(int fd)
{
    fd_ = fd;
}

Sockets::~Sockets()
{
    // printf("Socket fd : %d closed\n",fd_);
    close(fd_);
}

int Sockets::createTcpSock()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    return fd;
}

int Sockets::listen(int backlog)
{
    if (::listen(fd_, 128) < 0)
    {
        LOG_INFO("listen error!");
        return -1;
    }
    return 0;
}

int Sockets::bind(const InetAddress &addr)
{
    int opt = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (::bind(fd_, addr.get_sockaddr_Address(), sizeof(sockaddr)) < 0)
    {
        LOG_ERROR("Bind error: %s", strerror(errno));
        return -1;
    }
    // 监听的Fd在此赋值
    IP_ = addr.getIP();
    Port_ = addr.getPort();
    return 0;
}

int Sockets::accept(InetAddress &addr)
{
    socklen_t addrlen = sizeof(addr);
    int clientfd = ::accept(fd_, addr.get_sockaddr_Address(), &addrlen);
    return clientfd;
}

void Sockets::setIPandPort(const char* IP, uint16_t Port)
{
    IP_ = IP;
    Port_ = Port;
}

int Sockets::getfd() const
{
    return fd_;
}

std::string Sockets::getip() const
{
    return IP_;
}

uint16_t Sockets::getport() const
{
    return Port_;
}

void setUnblock(int sockfd)
{
    int flag = fcntl(sockfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flag);
}