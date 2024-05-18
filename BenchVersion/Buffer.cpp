#include "Buffer.h"


void Buffer::append(const char *data, int size)
{
    buf_.append(data, size);
}

void Buffer::appendwithsep(const char *data, size_t size)
{
    if(sep_ == 0 || sep_ == 2)
    {
        buf_.append(data, size);
    }
    else if(sep_== 1)
    {
        buf_.append((char *)&size, 4); // 添加报文长度
        buf_.append(data, size);       // 添加报文内容
    }


}

size_t Buffer::size()
{
    return buf_.size();
}

char *Buffer::data()
{
    return buf_.data();
}

void Buffer::clear()
{
    buf_.clear();
}

bool Buffer::empty()
{
    return buf_.empty();
}

void Buffer::erase(size_t pos, size_t n)
{
    buf_.erase(pos, n);
}

bool Buffer::pickmessage(std::string& ss)
{
    if(buf_.size() == 0) return false;

    if(sep_ == 0)
    {
        ss= buf_;
        buf_.clear();
    }
    else if (sep_ == 1)
    {
        int len;
        memcpy(&len,buf_.data(),4);             // 从buf_中获取报文头部。

        if (buf_.size()<len+4) return false;     // 如果buf_中的数据量小于报文头部，说明buf_中的报文内容不完整。

        ss=buf_.substr(4,len);                        // 从buf_中获取一个报文。
        buf_.erase(0,len+4);                          // 从buf_中删除刚才已获取的报文。
    }
    else if (sep_ == 2)
    {

        // strstr ---> 大字符串中匹配子字符串(遇到\0结束)
        // memmem ---> 大字符串中匹配子数据块(需要指定数据块大小)
        char* pt = strstr(buf_.data(), "\r\n");
        if (pt == nullptr)
        {
            return false;  // 没有完整的请求头
        }

        int reqLen = pt - buf_.data();
        ss = buf_.substr(0, reqLen + 2);  // 获取完整的请求头，包含\r\n
        buf_.clear();
    }

    return true;
}

