#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <stdint.h>
#include <strings.h>
#include <sys/stat.h>

class Buffer
{
private:
    std::string buf_;
    uint16_t sep_;  // 报文的分隔符：0-无分隔符(固定长度、视频会议)；1-四字节的报头；2-"\r\n\r\n"分隔符（http协议）

public:
    Buffer(uint16_t sep = 0) : sep_(sep) {}
    ~Buffer() {}

    void append(const char* data, int size);
    void appendwithsep(const char* data, size_t size);
    size_t size();
    char* data();
    void clear();
    bool empty();
    void erase(size_t pos, size_t n);  // 从pos位置开始删除n字节长度的内容

    bool pickmessage(std::string& ss);
};