# C++ High Performance Web Server With Muduo-like Framework



## Introduction

本项目是一个基于C++的高性能Web服务器，使用C++11编写，采用了多线程和非阻塞IO处理，以及Reactor事件处理模式，能够处理静态资源的请求，支持HTTP/1.1协议。服务器使用Epoll作为IO多路复用技术的核心，通过有效地管理socket连接和数据传输，提高了服务器的处理能力和响应速度



  
## Environment

- OS: Ubuntu 23.10
- Complier: G++ 13.2



  
## Usage

```
./server
```

( Before starting the server, modify configuration information such as the IP address and port in the "config.ini" file )



  
## Main Technical points

- **多线程处理**：利用线程池来管理和调度多个线程，充分提高多核CPU资源利用率和服务器响应速度，避免线程频繁创建销毁的开销。Acceptor分配在主线程上，主线程只负责accept请求，将Connection以Round Robin的方式分发给各IO线程（IO线程既负责IO通信也兼顾计算任务）
-  **非阻塞IO与Epoll**：采用非阻塞IO和Epoll事件循环，减少IO操作的等待时间，提升并发处理能力
- **Reactor模式**：服务器采用Reactor设计模式，将事件处理的主体和资源分离，提高系统的可扩展性和维护性
- **模块化设计**：服务器的核心功能模块化设计，包括网络连接处理、事件循环处理、HTTP请求解析和响应等，代码结构清晰，易于扩展和维护
- **配置文件支持**：服务器支持通过配置文件进行配置，方便根据不同环境进行定制化设置
- **服务器基础框架**：
  1. 若上层应用模块启用了工作线程池，则可以将IO通信与计算任务分离，工作线程池中的线程可以只专注于业务逻辑的处理计算，且使用了eventfd实现了线程之间的异步唤醒，将计算结果交给IO线程发回给客户端
  2. 使用智能指针等机制减少内存泄露的可能
  3. 服务器支持优雅关闭
  4. 实现了简单的Logger日志模块



  
## Code Structure Diagram

![](https://github.com/NatsumeMoMo/C-High-Performance-Web-Server-With-Muduo-like-Framewor/blob/main/pic/CodeStructure.png)

(实线和主干代表该类拥有指向的对象，可以掌管其生命周期。虚线代表的是只是为了使用其功能，由外界传入)



### Core components

- **TcpServer**：负责网络的监听和连接的建立，是服务器运行的起点
- **EventLoop**：事件循环处理器，负责处理所有的IO事件，是Reactor模式的核心
- **Channel**：表示一个IO事件，是文件描述符fd和其感兴趣的事件（如EPOLLIN、EPOLLOUT）的封装
- **Epoll**：封装了epoll的操作，用于注册、修改和删除事件，是非阻塞IO模型的关键
- **Connection**：表示客户端和服务器之间的连接，负责数据的读取和发送
- **Buffer**：数据缓冲区，用于存储从客户端读取的数据或待发送给客户端的数据
- **HttpRequest/HttpResponse**：处理HTTP请求和响应的逻辑，包括解析HTTP请求和生成HTTP响应
- **Logger**：日志模块，用于记录服务器的运行状态，帮助开发者进行问题诊断和性能调优
- **ThreadPool**：线程池，用于管理线程资源，提高服务器处理能力



### Flow of execution

1. 服务器启动，加载配置文件，初始化网络监听
2. 主线程创建EventLoop，负责处理连接事件，每个新连接分配给一个线程处理
3. 子线程中的EventLoop循环等待IO事件，使用Epoll监听
4. 当有数据到达时，Connection读取数据，HttpRequest解析请求，HttpResponse生成响应
5. 数据通过Connection发送回客户端，完成一次请求响应周期



  
## Stress Test

### Short link test

- 10线程 1000客户端 60s 短连接测试（QPS=13688.33）

  ![](https://github.com/NatsumeMoMo/C-High-Performance-Web-Server-With-Muduo-like-Framewor/blob/main/pic/shortlink.png))





### Long link test

- 10线程 1000客户端 60s 长连接测试（QPS=313253.87）

  ![](https://github.com/NatsumeMoMo/C-High-Performance-Web-Server-With-Muduo-like-Framewor/blob/main/pic/longlink.png)







  
## Thoughts

在人手一个Muduo库的24年，我依旧选择了基于Muduo的WebServer这样一个烂大街的项目来作为自己的第一个实战练手。因为虽然它烂大街了，但不得不承认这样的一个WebServer是综合掌握Linux网络编程、高并发编程、Reactor模型等各方面知识点的一个优秀起步项目，我个人认为也是每个打算走CPP后端开发方向的程序员必做的项目之一。
