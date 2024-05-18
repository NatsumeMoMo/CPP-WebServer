#pragma once
#include <sys/stat.h>
#include <cstring>
#include <strings.h>
#include <iostream>
#include <string>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <functional>
#include <memory>
#include <map>

#include "HttpResponse.h"
#include "Connection.h"
#include "Buffer.h"

/*
enum class 避免了与其他作用域中的名称冲突、提供了更强的类型安全、
允许显式地指定底层类型，这有助于控制枚举类型的大小和行为（PrecessState:char）
*/
enum class ProcessState : char
{
    ParseReqLine,
    ParseReqHeaders,
    ParseReqBody,
    ParseReqDone
};

class HttpRequest
{
public:
    HttpRequest(HttpResponse *response); // 由HttpServer传入
    ~HttpRequest();

    /* 解析请求行: 方法类型（GET）、请求的资源的URI（统一资源标识符）和HTTP版本
        GET /index.html HTTP/1.1
        POST /login HTTP/1.1
     */
    char *splitRequestLine(const char *start, const char *end, const char *sub,
                           std::function<void(std::string)> callback);
    bool parseRequestLine();

    /*
        解析请求头. 该函数处理请求头中的一行，如果要处理多行就多次调用该函数即可
    */
    void addHeader(const std::string key, const std::string value);
    bool parseRequestHeader();

    // 解析Http请求
    bool parseHttpRequest(std::string &message, std::shared_ptr<Connection> connect);

    // 处理Http请求协议
    int processHttpRequest();

    int sendFile(std::string fileName, std::shared_ptr<Connection> connect);
    int sendDir(std::string filename, std::shared_ptr<Connection> connect);
    const std::string getFileType(const std::string name);
    int hexToDec(char c);
    std::string decodeMsg(std::string msg);

    /*
        浏览器和客户端建立连接之后进行了多次通信，就需要多次往该HttpRequest中写入数据
        进行数据写入的时候就要把上一次的数据清除掉
    */
    void reset();

    void setMethod(std::string method) { method_ = method; }
    void setUrl(std::string url) { url_ = url; }
    void setVersion(std::string version) { version_ = version; }
    void setState(ProcessState state) { curState_ = state; }
    void setOnMessageCallbakc(std::function<void(std::shared_ptr<Connection>, std::string&)> cb)
    {
        onmessagecallback_ = cb;
    }

private:
    HttpResponse *response_;
    Buffer buf;
    std::function<void(std::string)> methodcallback_;
    std::function<void(std::string)> urlcallback_;
    std::function<void(std::string)> versioncallback_;
    /* 回调HttpServer的OnMessage() */
    std::function<void(std::shared_ptr<Connection>, std::string&)> onmessagecallback_;
    /*
        GET /index.html HTTP/1.1
        Host: 127.0.0.1:8080
        Connection: keep-alive
        Upgrade-Insecure-Requests: 1
        .....
        除了第一行的GET请求，剩下的都可以看做键值对
    */
    std::map<std::string, std::string> headers_;
    std::string method_;
    std::string url_;
    std::string version_;
    ProcessState curState_;
};
