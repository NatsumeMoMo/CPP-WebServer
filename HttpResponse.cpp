#include "HttpResponse.h"

HttpResponse::HttpResponse()
{
    statusCode_ = StatusCode::Unknown;
    headers_.clear();
    filename_ = std::string();
    senddatacallback_= nullptr;

    
}

void HttpResponse::addHeader(const std::string key, const std::string value)
{
    if (key.empty() || value.empty())
    {
        LOG_INFO("HttpResponse key or value is empty");
        return;
    }
    headers_[key] = value;
}

void HttpResponse::prepareMsg(std::shared_ptr<Connection> connect)
{
    // 使用 std::string 来构建响应头
    std::string buffer;
    char tmp[4096] = {0};
    int code = static_cast<int>(statusCode_);

    // 状态行
    sprintf(tmp, "HTTP/1.1 %d %s\r\n", code, info_.at(code).c_str());
    buffer.append(tmp);

    // 响应头
    for (const auto& header : headers_)
    {
        sprintf(tmp, "%s: %s\r\n", header.first.c_str(), header.second.c_str());
        buffer.append(tmp);
    }

    // 添加空行，标识响应头的结束, 一定要注意不能少了这步
    buffer.append("\r\n");

    // 发送响应头
    onmessagecallback_(connect, buffer);

    // 发送响应体
    if (senddatacallback_)
    {
        senddatacallback_(filename_, connect);
    }

}