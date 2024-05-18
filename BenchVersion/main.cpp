#include "HttpServer.h"
#include <signal.h>
#include <sstream>
#include <map>
#include <fstream>
#include <string>
#include <stdexcept>

HttpServer *mServer;

std::map<std::string, std::string> load_ini(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    while (getline(file, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (getline(is_line, key, '=')) {
            std::string value;
            if (getline(is_line, value)) {
                config[key] = value;
            }
        }
    }

    return config;
}

void Stop(int sig)
{
    printf("sig= %d\n", sig);
    mServer->Stop();
    delete mServer;
    printf("Server已终止\n");
    exit(0);
}

int main()
{
    signal(SIGPIPE, SIG_IGN); // 忽略Broken pipe信号
    std::map<std::string, std::string> config;
    uint16_t port;
    try
    {
        config = load_ini("/home/jason/shared/Work_and_learn/Code/CPP/CPPwork/WebServer/MyWebServer/config.ini");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    try {
        unsigned long temp = std::stoul(config["Port"]);
        if (temp > 65535) {
            throw std::out_of_range("数值超出 uint16_t 范围");
        }
        port = static_cast<uint16_t>(temp);
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }


    mServer = new HttpServer(config["IP"], port);
    signal(SIGTERM, Stop); // 信号15，kill -15 PID 系统kill和kiilall 命令默认发送的信号
    signal(SIGINT, Stop);  // 信号2，按Ctrl+C发送的信号

    chdir("/home/jason/shared/Work_and_learn/Code/CPP/CPPwork/WebServer/MyWebServer/websrc"); // 切换服务器的工作路径 <unistd.h>
    mServer->Start();
    return 0;
}

/*
TODO:
1. 使用配置文件启动服务器

  cd ../WebServer/WebBench

  ./webbench -c 1000 -t 10 -2 http://localhost:8888/
  ./webbench -c 1000 -t 30 -2 http://localhost:8888/
  ./webbench -c 1000 -t 30 -2 -f http://localhost:8888/
  ./webbench -c 1000 -t 60 -2 -f http://localhost:8888/
-- 长连接测试：
  ./webbench -c 1000 -t 30 -k -2 http://localhost:8888/
  ./webbench -c 1000 -t 60 -k -2 http://localhost:8888/


  ./SimpleHttp.out 8888 /home/jason/shared/Work_and_learn/Code/CPP/CPPwork/WebServer/MyWebServer/websrc
*/