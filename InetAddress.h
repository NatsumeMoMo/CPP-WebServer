#pragma once

#include <arpa/inet.h>
#include <string>

class InetAddress
{
public:
    InetAddress() {}
    InetAddress(const char* IP, uint16_t port); //服务器监听用
    InetAddress(sockaddr_in addr);


    const char* getIP() const;
    uint16_t getPort() const; 
    sockaddr* get_sockaddr_Address() const;
    socklen_t getAddrSize() const;
    
    sockaddr_in get_sockaddr_in() const;

private:
    sockaddr_in addr_;
};

