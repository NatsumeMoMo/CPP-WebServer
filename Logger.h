#pragma once
#include <iostream>
#include <string>

#include "Timestamp.h"

// LOG_INFO("%s %d, arg1, arg2")
/* 当宏涉及多行代码时，需要使用do while(0), 防止产生错误 */
#define LOG_INFO(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_ERROR(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_FATAL(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

/* 一般不会输出DEBUG级别的日志, 因为DEBUG信息比较多, 打印出来会影响查看正常流程
    而且运行的过程中不断的打印调试信息会影响效率, 所以一般默认关闭 */
#ifdef DEBUG_
#define LOG_DEBUG(logmsgFormat, ...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)
#else // 没定义的话相当于LOG_DEBUG什么都不输出
#define LOG_DEBUG(logmsgFormat, ...)
#endif
// 定义日志的级别 INFO ERROR FATAL DEBUG

enum LogLevel {
    INFO, // 普通信息
    ERROR, // 错误信息
    FATAL, // core dump信息
    DEBUG // 调试信息
};

// 输出一个日志类
class Logger
{
public:
    static Logger& instance();

    // 设置日志级别
    void setLevel(LogLevel level)
    {
        level_ = level;
    }

    void log(std::string msg);
private:
    LogLevel level_;
    Logger() {}
};