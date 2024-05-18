#include "Timestamp.h"


Timestamp Timestamp::now()
{
    return Timestamp(); // 返回当前时间
}

/*
1、buf 作为一个 char[32] 类型，其实在此上下文中被自动转换成指向首元素的指针 char*。
2、std::string 的构造函数接收这个 char*，它指向的是一个以 null 结尾的字符串（因为 snprintf 保证字符串以 null 结尾）。
3、使用 char* 指针构建一个新的 std::string 对象，该对象包含与 char* 相同的字符序列。
4、返回的是这个新构造的 std::string 对象。
*/
std::string Timestamp::tostring() const
{
    char buf[32];
    tm *tm_time = localtime(&secsinceepoch_);
    snprintf(buf, 20, "%4d-%02d-%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900, 
             tm_time->tm_mon + 1, 
             tm_time->tm_mday,
             tm_time->tm_hour, 
             tm_time->tm_min, 
             tm_time->tm_sec);
    return buf;
}