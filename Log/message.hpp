#pragma once
#include "util.hpp"
#include "level.hpp"
#include <string>
#include <thread>
namespace nhylog
{

struct LogMsg
{
    LogLevel::Level level_;
    time_t ctime_;
    size_t line_;
    std::thread::id tid_;
    std::string file_;
    std::string logger_;
    std::string payload_;

    LogMsg(
        LogLevel::Level level,
        size_t line,
        const std::string file,
        const std::string logger,
        const std::string payload
    ) : 
        level_(level),
        ctime_(util::Date::now()),
        line_(line),
        tid_(std::this_thread::get_id()),
        file_(file),
        logger_(logger),
        payload_(payload)
    {

    }
};// LogMsg end




}// nhylog end