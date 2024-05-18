#include "HttpServer.h"

// -------------------------------------------

// int sendDIR(const char *dirName, int cfd)
// {
//     char buf[4096] = {0};
//     sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName);
//     struct dirent **namelist;
//     int num = scandir(dirName, &namelist, NULL, alphasort);
//     for (int i = 0; i < num; ++i)
//     {
//         // 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
//         char *name = namelist[i]->d_name;
//         struct stat st;
//         char subPath[1024] = {0};
//         sprintf(subPath, "%s/%s", dirName, name);
//         printf("SubPath is: %s\n", subPath);
//         stat(subPath, &st);
//         if (S_ISDIR(st.st_mode))
//         {
//             // a标签 <a href="">name</a>
//             sprintf(buf + strlen(buf),
//                     "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
//                     name, name, st.st_size);
//         }
//         else
//         {
//             sprintf(buf + strlen(buf),
//                     "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
//                     name, name, st.st_size);
//         }
//         send(cfd, buf, strlen(buf), 0);
//         memset(buf, 0, sizeof(buf));
//         free(namelist[i]);
//     }
//     sprintf(buf, "</table></body></html>");
//     send(cfd, buf, strlen(buf), 0);
//     free(namelist);
//     return 0;
// }

// int sendHeadMsg(int cfd, int status, const char* descr, const char* type, int length)
// {
//     // 状态行
//     char buf[4096] = { 0 };
//     sprintf(buf, "http/1.1 %d %s\r\n", status, descr);
//     // 响应头
//     sprintf(buf + strlen(buf), "content-type: %s\r\n", type);
//     sprintf(buf + strlen(buf), "content-length: %d\r\n\r\n", length);

//     send(cfd, buf, strlen(buf), 0);
//     return 0;
// }

HttpServer::HttpServer(const std::string IP, const uint16_t Port) : TcpServer(new Server(IP.c_str(), Port))
{
    TcpServer->setEchoNewConnectionCallback(std::bind(&HttpServer::HandleNewConnection, this, std::placeholders::_1));
    TcpServer->setEchoCloseCallback(std::bind(&HttpServer::HandleCloseConnection, this, std::placeholders::_1));
    TcpServer->setEchoErrorCallback(std::bind(&HttpServer::HandleErrorConnection, this, std::placeholders::_1));
    TcpServer->setEchoMessageCallback(std::bind(&HttpServer::HandleMessage, this, std::placeholders::_1,
                                                std::placeholders::_2));
    TcpServer->setEchoSendCompleteCallback(std::bind(&HttpServer::HandleSendMessageNotify, this,
                                                     std::placeholders::_1));
    TcpServer->setEchoTimeoutCallback(std::bind(&HttpServer::HandleTimeout, this, std::placeholders::_1));
}

HttpServer::~HttpServer()
{
    delete TcpServer;
    printf("TcpServer已停止。\n");
}

void HttpServer::Start()
{
    TcpServer->serverLoop();
}

void HttpServer::Stop()
{
    TcpServer->stop();
}

// 处理新客户端连接请求后的业务需求，在TcpServer类中回调此函数。
void HttpServer::HandleNewConnection(std::shared_ptr<Connection> connect)
{

    // 根据业务的需求，在这里可以增加其它的代码。
}

// 关闭新客户端连接请求后的业务需求，在TcpServer类中回调此函数。
void HttpServer::HandleCloseConnection(std::shared_ptr<Connection> connect)
{
    // 根据业务的需求，在这里可以增加其它的代码。
}

void HttpServer::HandleErrorConnection(std::shared_ptr<Connection> connect)
{
    // 根据业务的需求，在这里可以增加其它的代码。
    printf("client which fd is %d connect error\n", connect->getFd());
}

// 客户端的连接错误后的业务处理，在TcpServer类中回调此函数。
void HttpServer::HandleMessage(std::shared_ptr<Connection> connect, std::string &message)
{
    int cfd = connect->getFd();
    sendHelloWorld(connect);

    // parseRequestLine(message.data(), connect);
}

// 处理客户端的请求报文，用于添加给线程池
void HttpServer::OnMessage(std::shared_ptr<Connection> connect,const std::string &message)
{
    connect->sendMessageInLoop(message.data(), message.size()); // 发送缓冲区中的数据

}

// 数据发送完成后的业务处理，在TcpServer类中回调此函数。
void HttpServer::HandleSendMessageNotify(std::shared_ptr<Connection> connect)
{
    // 根据业务的需求，在这里可以增加其它的代码。
}

// epoll_wait超时后的业务处理，在TcpServer类中回调此函数。
void HttpServer::HandleTimeout(EventLoop *loop)
{
    // 根据业务的需求，在这里可以增加其它的代码。
    printf("epoll_wait() timeout.\n");
}

// int HttpServer::parseRequestLine(const char *line, std::shared_ptr<Connection> connect)
// {
//     // 解析请求行 get /xxx/1.jpg HTTP/1.1
//     char method[12];
//     char path[1024];
//     char buf[4096] = {0};
//     // std::string path;
//     std::string retMsg;
//     sscanf(line, "%[^ ] %[^ ]", method, path);
//     if (strcasecmp(method, "get") != 0) // 只支持get请求, 不区分大小写的比较
//     {
//         return -1;
//     }
//     decodeMsg(path, path);
//     char *file = nullptr;
//     if (strcasecmp(path, "/") == 0)
//     {
//         file = "./";
//     }
//     else
//     {
//         file = path + 1;
//     }
//     // 获取文件属性
//     struct stat st;

//     int ret = stat(file, &st);
//     if (ret == -1)
//     {
//         // 文件不存在，回复404页面，长度制定为-1，让浏览器自己获取
//         responseHeadMsg(404, "Not Found", getFileType(".html"), -1, connect);
//         sendFile("404.html", connect->getFd());// 将指定资源目录下的404.html发送过去

//         return 0;
//     }
//     // 判断文件类型
//     if (S_ISDIR(st.st_mode))
//     {
//         // 目录, 把目录中的内容发送给客户端
//         responseHeadMsg(200, "OK", getFileType(".html"), -1, connect);
//         sendDir(file, connect);
//     }
//     else
//     {
//         // 文件
//         // 把文件的内容发送给客户端
//         // responseHeadMsg(200, "OK", getFileType(file), st.st_size, connect);
//         sendHeadMsg(connect->getFd(), 200, "OK", getFileType(file), st.st_size);
//         sendFile(file, connect->getFd());
//     }

//     return 0;
// }


// int HttpServer::sendFile(const char *fileName, int cfd)
// {
//     // 1. 打开文件
//     int fd = open(fileName, O_RDONLY);
//     assert(fd > 0);

//     off_t offset = 0;
//     int size = lseek(fd, 0, SEEK_END);
//     lseek(fd, 0, SEEK_SET);

//     while (offset < size) {
//         int ret = sendfile(cfd, fd, &offset, size - offset);
//         if (ret == -1) {
//             if (errno == EAGAIN) {
//                 // 非阻塞情况下没有数据可发送
//                 usleep(10000); // 暂停一段时间
//             } else if (errno == EPIPE) {
//                 // 客户端已经关闭连接
//                 break;
//             } else {
//                 perror("sendfile");
//                 break;
//             }
//         }
//     }

//     close(fd);
//     return 0;

// }


// int HttpServer::sendDir(std::string filename, std::shared_ptr<Connection> connect)
// {
//      char buf[4096] = {0};
//     sprintf(buf, "<html><head><title>%s</title></head><body><table>", filename.c_str());
//     struct dirent **namelist;
//     int n = scandir(filename.data(), &namelist, NULL, alphasort);
//     for (int i = 0; i < n; i++)
//     {
//         // 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
//         // 得到的name只是文件名, 要判断其是否为目录就要获得其相对路径
//         char *name = namelist[i]->d_name;
//         char subPath[1024] = {0};
//         sprintf(subPath, "%s/%s", filename.data(), name);
//         struct stat st;
//         stat(subPath, &st);
//         if (S_ISDIR(st.st_mode))
//         {
//             // 目录
//             // 标签a <a href="">name</a>
//             sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
//                     name, name, st.st_size);
//         }
//         else
//         {
//             // 文件
//             sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
//                     name, name, st.st_size);
//         }
//         // 发送目录或文件名
//         /*可以直接将 std::string 对象作为参数传递给 test()，也可以创建一个临时的 std::string 对象来进行转换。
//          * 由于函数参数 std::string& 是一个非 const 的引用，因此不能传递临时对象（例如字符串字面量或返回的临时
//          * 字符串对象）。如果你想直接传递临时对象，则应使用 const std::string& 参数。*/
//         OnMessage(connect, std::string(buf));
//         memset(buf, 0, sizeof(buf));
//         free(namelist[i]);
//     }
//     sprintf(buf, "</table></body></html>");
//     OnMessage(connect, std::string(buf));
//     free(namelist);
//     return 0;
// }

// const char *HttpServer::getFileType(const char *name)
// {
//     // a.jpg a.mp4 a.html
//     // 自右向左查找‘.’字符, 如不存在返回NULL
//     const char *dot = strrchr(name, '.');
//     if (dot == NULL)
//         return "text/plain; charset=utf-8"; // 纯文本
//     if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
//         return "text/html; charset=utf-8";
//     if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
//         return "image/jpeg";
//     if (strcmp(dot, ".gif") == 0)
//         return "image/gif";
//     if (strcmp(dot, ".png") == 0)
//         return "image/png";
//     if (strcmp(dot, ".css") == 0)
//         return "text/css";
//     if (strcmp(dot, ".au") == 0)
//         return "audio/basic";
//     if (strcmp(dot, ".wav") == 0)
//         return "audio/wav";
//     if (strcmp(dot, ".avi") == 0)
//         return "video/x-msvideo";
//     if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
//         return "video/quicktime";
//     if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
//         return "video/mpeg";
//     if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
//         return "model/vrml";
//     if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
//         return "audio/midi";
//     if (strcmp(dot, ".mp3") == 0)
//         return "audio/mpeg";
//     if (strcmp(dot, ".ogg") == 0)
//         return "application/ogg";
//     if (strcmp(dot, ".pac") == 0)
//         return "application/x-ns-proxy-autoconfig";

//     return "text/plain; charset=utf-8";
// }

// int HttpServer::hexToDec(char c)
// {
//     if (c >= '0' && c <= '9')
//         return c - '0';
//     if (c >= 'a' && c <= 'f')
//         return c - 'a' + 10;
//     if (c >= 'A' && c <= 'F')
//         return c - 'A' + 10;

//     return 0;
// }

// void HttpServer::decodeMsg(char *to, char *from)
// {
//     for (; *from != '\0'; ++to, ++from)
//     {
//         // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
//         // Linux%E5%86%85%E6%A0%B8.jpg
//         if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
//         {
//             // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
//             // B2 == 178
//             // 将3个字符, 变成了一个字符, 这个字符就是原始数据
//             *to = hexToDec(from[1]) * 16 + hexToDec(from[2]);

//             // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
//             from += 2;
//         }
//         else
//         {
//             // 字符拷贝, 赋值
//             *to = *from;
//         }
//     }
//     *to = '\0';
// }


// void HttpServer::responseHeadMsg(int status, const char *descr,
//                                  const char *type, int length, std::shared_ptr<Connection> connect)
// {
//     // 使用 std::string 来构建响应头
//     // 状态行
//     std::string buf;
//     char temp[4096] = {0};
//     sprintf(temp, "http/1.1 %d %s\r\n", status, descr);
//     // 响应头
//     sprintf(temp + strlen(temp), "content-type: %s\r\n", type);
//     sprintf(temp + strlen(temp), "content-length: %d\r\n\r\n", length);

//     buf = temp;
//     OnMessage(connect, buf);
// }

void HttpServer::sendHelloWorld(std::shared_ptr<Connection> connect)
{
    std::string helloWorldMessage = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Content-Length: 11\r\n"
                                    "\r\n"
                                    "Hello World";

    connect->sendMessageInLoop(helloWorldMessage.data(), helloWorldMessage.size());
}