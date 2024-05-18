#include "InetAddress.h"

InetAddress::InetAddress(const char* IP, uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(IP);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const sockaddr_in addr) : addr_(addr)
{

}

const char* InetAddress::getIP() const 
{
    return inet_ntoa(addr_.sin_addr);
}

uint16_t InetAddress::getPort() const
{
    return ntohs(addr_.sin_port);
}

sockaddr* InetAddress::get_sockaddr_Address() const
{
    return (sockaddr*) &addr_;
}

socklen_t InetAddress::getAddrSize() const
{
    return sizeof(addr_);
}

sockaddr_in InetAddress::get_sockaddr_in() const
{
    return addr_;
}