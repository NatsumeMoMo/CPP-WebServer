#include <signal.h>
#include <sstream>
#include <map>
#include <fstream>
#include <string>

#include "HttpServer.h"
#include "Logger.h"

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

std::string get_current_directory() {
    std::string path = __FILE__;  // 获取当前源文件的完整路径
    std::size_t last_slash_pos = path.find_last_of("/\\");  // 查找最后一个斜杠的位置
    return path.substr(0, last_slash_pos);  // 提取目录路径
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


    mServer = new HttpServer(config["IP"], port, std::stoi(config["Sep"]));
    signal(SIGTERM, Stop); // 信号15，kill -15 PID 系统kill和kiilall 命令默认发送的信号
    signal(SIGINT, Stop);  // 信号2，按Ctrl+C发送的信号

    // 获取当前文件的目录路径
    std::string current_directory = get_current_directory();
    std::string target_path = current_directory + "/websrc";

    // 切换服务器的工作路径
    if (chdir(target_path.c_str()) != 0) {
        LOG_ERROR("无法切换工作路径到: %s", target_path.c_str());
        return 1;
    }
    mServer->Start();
    return 0;
}
