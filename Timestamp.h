#pragma once

#include <iostream>
#include <string>
#include <ctime>

class Timestamp
{

private:
    time_t secsinceepoch_;  // 整数表示的时间（从1970到现在已逝去的秒数）

public:
    Timestamp() : secsinceepoch_(time(0)) {}
    Timestamp(time_t sec) : secsinceepoch_(sec) {}

    time_t toint() const { return secsinceepoch_; }
    std::string tostring() const;

    static Timestamp now();
};