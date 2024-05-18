#pragma once
#include<iostream>
#include<cstring>
#include <sys/socket.h>
#include <string>
#include <map>
#include <memory>

#include "Connection.h"


// 定义状态码枚举
enum class StatusCode
{
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFound = 404
};


class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse(){};

    void addHeader(const std::string key, const std::string value);
    void prepareMsg(std::shared_ptr<Connection> connect);
    inline void setFileName(std::string filename) { filename_ = filename; }
    inline void setStatusCode(StatusCode code) { statusCode_ = code; }
    inline void setSendDataCallback(std::function<void(std::string, std::shared_ptr<Connection>)> cb) { senddatacallback_ = cb; }
    inline void setOnMessageCallbakc(std::function<void(std::shared_ptr<Connection>, std::string&)> cb)
    {
        onmessagecallback_ = cb;
    }
private:
    StatusCode statusCode_;
    std::string filename_;
    std::map<std::string, std::string> headers_;
    std::map<int, std::string> info_ = {
        {200, "OK"},
        {301, "Moved Permanently"},
        {302, "Moved Temporarily"},
        {400, "Bad Request"},
        {404, "Not Found"}
    };
    /* 回调HttpServer的OnMessage() */
    std::function<void(std::shared_ptr<Connection>, std::string&)> onmessagecallback_;
    /* 回调HttpRequest的sendDir() or sendFile() */
    std::function<void(std::string, std::shared_ptr<Connection>)> senddatacallback_;
};
