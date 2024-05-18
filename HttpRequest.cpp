#include "HttpRequest.h"


HttpRequest::HttpRequest(HttpResponse* response) : response_(response)
{
    reset();
}

HttpRequest::~HttpRequest()
{

}

void HttpRequest::reset()
{
    curState_ = ProcessState::ParseReqLine;
    method_ = url_ = version_ = std::string();
    headers_.clear();
}

char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, std::function<void(std::string)> callback)
{
   const char* space = end;
   if(sub != nullptr)
   {
       space = static_cast<const char*>(memmem(start, end - start, sub, strlen(sub)));
       assert(space != nullptr);
   }
   int length = space - start;
   callback(std::string(start, length));
   return const_cast<char*>(space + 1);
}


bool HttpRequest::parseRequestLine()
{
    /*
     * buf.data() 返回的 const char* 指针赋值给一个 const char* 类型的变量，
     * 然后在需要修改的地方使用 const_cast<char*> 将其转换为 char*。
     * */
    const char* end = strstr(buf.data(), "\r\n");
    const char* start = buf.data();

    int lineLength = end - buf.data();
    if(lineLength > 0)
    {
        methodcallback_ = std::bind(&HttpRequest::setMethod, this, std::placeholders::_1);
        start = splitRequestLine(start, end, " ", methodcallback_);

        urlcallback_ = std::bind(&HttpRequest::setUrl, this, std::placeholders::_1);
        start = splitRequestLine(start, end, " ", urlcallback_);

        versioncallback_ = std::bind(&HttpRequest::setVersion, this, std::placeholders::_1);
        start = splitRequestLine(start, end, nullptr, versioncallback_);

        buf.erase(0, lineLength + 2); // 删除已经解析的请求行, 为解析请求头做准备
        setState(ProcessState::ParseReqHeaders); // 切换状态
        return true;
    }
    return false;

}

void HttpRequest::addHeader(const std::string key, const std::string value)
{
    if (key.empty() || value.empty())
    {
        LOG_INFO("HttpRequest key or value is empty");
        return;
    }
    headers_[key] = value;
}

bool HttpRequest::parseRequestHeader()
{
    char* pt = strstr(buf.data(), "\r\n"); // 键值对的末尾依旧是 \r\n
    if (pt == nullptr)
    {
        return false;  // 没有完整的请求头
    }
    char* start = buf.data();
    int lineLength = pt - buf.data();
    char* middle = static_cast<char*>(memmem(start, lineLength, ": ", 2));
    if (middle != nullptr)
    {
        int keyLen = middle - start;
        int valueLen = pt - middle - 2;
        if (keyLen > 0 && valueLen > 0)
        {
            std::string key(start, keyLen);
            std::string value(middle + 2, valueLen);
            addHeader(key, value);
        }
        // 移动读数据的位置
        buf.erase(0, lineLength + 2);
    }
    else
    {
        // 请求头被解析完了, 跳过空行
        buf.erase(0, 2);
        // 修改解析状态
        // 忽略 post 请求, 按照 get 请求处理
        setState(ProcessState::ParseReqDone);
    }
    return true;
}

bool HttpRequest::parseHttpRequest(std::string &message, std::shared_ptr<Connection> connect)
{
    buf.append(message.data(),message.size());
    bool flag = true;
    while (curState_ != ProcessState::ParseReqDone)
    {
        switch (curState_)
        {
            case ProcessState::ParseReqLine:
                flag = parseRequestLine();
                break;
            case ProcessState::ParseReqHeaders:
                flag = parseRequestHeader();
                break;
            case ProcessState::ParseReqBody:
                break;
            default:
                break;
        }
        if (!flag)
        {
            return flag;
        }
        // 判断是否解析完毕了, 如果完毕了, 需要准备回复的数据
        if (curState_ == ProcessState::ParseReqDone)
        {
            // 1. 根据解析出的原始数据, 对客户端的请求做出处理
            processHttpRequest();
            // 2. 组织响应数据并发送给客户端
            response_->prepareMsg(connect);
        }
    }
    curState_ = ProcessState::ParseReqLine;   // 状态还原, 保证还能继续处理第二条及以后的请求
    return flag;
}

int HttpRequest::processHttpRequest()
{

    if(strcasecmp(method_.data(), "GET") != 0)
    {
        return -1;
    }

    url_ = decodeMsg(url_);
    const char *file = nullptr;
    if(strcasecmp(url_.data(), "/") == 0)
    {
        file = "./";
    }
    else
    {
        file = url_.data() + 1;
    }
    // 获取文件属性
    struct stat fileStat;
    int ret = stat(file, &fileStat);
    // 文件不存在, 返回404
    if(ret == -1)
    {
        response_->setFileName("404.html");
        response_->setStatusCode(StatusCode::NotFound);

        // 响应头
        response_->addHeader("Content-Type", getFileType(".html"));
        response_->setSendDataCallback(std::bind(&HttpRequest::sendFile, this, std::placeholders::_1, std::placeholders::_2));
        return 0;
    }
    // 文件存在则设置文件名, 供HttpResponse回调HttpRequest的sendDir() or sendFile() 时使用
    response_->setFileName(file);
    response_->setStatusCode(StatusCode::OK);
    // 判断文件类型, 如果是目录
    if(S_ISDIR(fileStat.st_mode))
    {
        // 响应头
        response_->addHeader("Content-Type", getFileType(".html"));
        response_->setSendDataCallback(std::bind(&HttpRequest::sendDir, this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        // 响应头
        response_->addHeader("Content-Type", getFileType(file));
        response_->addHeader("Content-Length", std::to_string(fileStat.st_size));
        response_->setSendDataCallback(std::bind(&HttpRequest::sendFile, this, std::placeholders::_1, std::placeholders::_2));
    }

    return 0;
}


int HttpRequest::sendDir(std::string filename, std::shared_ptr<Connection> connect)
{

    char buf[4096] = {0};
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", filename.c_str());
    struct dirent **namelist;
    int n = scandir(filename.data(), &namelist, NULL, alphasort);
    std::string message;
    for (int i = 0; i < n; i++)
    {
        // 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
        // 得到的name只是文件名, 要判断其是否为目录就要获得其相对路径
        char *name = namelist[i]->d_name;
        char subPath[1024] = {0};
        sprintf(subPath, "%s/%s", filename.data(), name);
        struct stat st;
        stat(subPath, &st);
        if (S_ISDIR(st.st_mode))
        {
            // 目录
            // 标签a <a href="">name</a>
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
                    name, name, st.st_size);
        }
        else
        {
            // 文件
            sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                    name, name, st.st_size);
        }
        // 发送目录或文件名
        /*可以直接将 std::string 对象作为参数传递给 test()，也可以创建一个临时的 std::string 对象来进行转换。
         * 由于函数参数 std::string& 是一个非 const 的引用，因此不能传递临时对象（例如字符串字面量或返回的临时
         * 字符串对象）。如果你想直接传递临时对象，则应使用 const std::string& 参数。*/
        message.append(buf);
        onmessagecallback_(connect, message);
        memset(buf, 0, sizeof(buf));
        message.clear();
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    message.append(buf);
    onmessagecallback_(connect, message);
    free(namelist);
    return 0;
}

int HttpRequest::sendFile(std::string fileName, std::shared_ptr<Connection> connect)
{
    // 1. 打开文件
    int fd = open(fileName.data(), O_RDONLY);
    assert(fd > 0);

    off_t offset = 0;
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    while (offset < size) {
        int ret = sendfile(connect->getFd(), fd, &offset, size - offset);
        if (ret == -1) {
            if (errno == EAGAIN) {
                // 非阻塞情况下没有数据可发送
                usleep(10000); // 暂停一段时间
            } else if (errno == EPIPE) {
                // 客户端已经关闭连接
                break;
            } else {
                LOG_ERROR("Sendfile error: %s", strerror(errno));
                break;
            }
        }
    }

    close(fd);
    return 0;
}

std::string HttpRequest::decodeMsg(std::string msg)
{
    std::string str = std::string();
    const char* from = msg.data();
    for(; *from != '\0'; from++)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if(from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            str.append(1, *from);
        }
    }
    str.append(1, '\0');
    return str;
}

// 将字符转换为整形数
int HttpRequest::hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

const std::string HttpRequest::getFileType(const std::string name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL
    const char* dot = strrchr(name.data(), '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}