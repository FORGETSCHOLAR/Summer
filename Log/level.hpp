#pragma once


namespace nhylog
{
class LogLevel
{
public:
    enum class Level{
        UNKNOW = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };

    static const char* toString(LogLevel::Level level)
    {
        switch (level)
        {
            case LogLevel::Level::DEBUG : return "DEBUG";
            case LogLevel::Level::INFO : return "INFO";
            case LogLevel::Level::WARN : return "WARN";
            case LogLevel::Level::ERROR : return "ERROR";
            case LogLevel::Level::FATAL : return "FATAL";
            case LogLevel::Level::OFF : return "OFF";

        }

        return "UNKNOW";
    }

};
}// nhylog end...