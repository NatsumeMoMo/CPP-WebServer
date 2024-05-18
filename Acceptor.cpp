#include "Acceptor.h"

Acceptor::Acceptor(EventLoop *loop, const char *IP, uint16_t Port) : Loop_(loop)
{
    ServerSocket = new Sockets();
    Server_addr = new InetAddress(IP, Port);
    ServerSocket->bind(*Server_addr);
    ServerSocket->listen(128);
    acceptorChannel = new Channel(Loop_->getEpoll(), ServerSocket->getfd());
    acceptorChannel->setReadCallback(std::bind(&Acceptor::newConnection,
                                               this));
    acceptorChannel->enableReading();
}

Acceptor::~Acceptor()
{
    // 由于Acceptor中的EventLoop不属于Acceptor对象，所以不能在此处释放
    delete ServerSocket;
    delete Server_addr;
    delete acceptorChannel;
}

void Acceptor::newConnection()
{
    InetAddress clinetAddr;
    Sockets *clientSocket = new Sockets(ServerSocket->accept(clinetAddr));
    setUnblock(clientSocket->getfd());
    clientSocket->setIPandPort(clinetAddr.getIP(), clinetAddr.getPort());

    /*
    在Reactor模型中，Connection和Acceptor是平级的关系，Connection应该为TcpServer类所有
    因此应该在TcpServer类中创建。TcpServer中的 newConnection() 函数只需要建立Connection
    类的这一行代码即可。这里执行 new Connection 的回调。之前的接受新客户端的代码不应该放到
    TcpServer中，因为接受新客户端本就应该是接收器的工作
    */
    /* TcpServer::newConnectioon-> Acceptor::setCallback->new Connection */
    connectCallback_(clientSocket);
}

void Acceptor::setConnectionCallback(std::function<void(Sockets *)> cb)
{
    connectCallback_ = cb;
}