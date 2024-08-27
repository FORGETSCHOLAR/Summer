#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <memory>

#include "util.hpp"

/*
    设计落地方向
*/

namespace nhylog
{

class LogSink
{
public:
    using ptr = std::shared_ptr<LogSink>;

    LogSink() = default;
    virtual ~LogSink(){}
    
    virtual void log(const char* data,size_t len) = 0;
};//class LogSink end


class StdoutSink : public LogSink
{
public:
    virtual void log(const char* data,size_t len) override
    {
        std::cout.write(data,len);
    }
};//class StdoutSink end


class FileSink : public LogSink
{
public:
    FileSink(const std::string& pathname) : pathname_(pathname)
    {
        util::File::createDir(util::File::path(pathname_));
        ofs_.open(pathname_,std::ios::binary | std::ios::app);
    }

    virtual void log(const char* data,size_t len) override
    {
        ofs_.write(data,len);
        assert(ofs_.good());
    }
private:
    std::string pathname_;
    std::ofstream ofs_;
};// class FileSink end

class RollBySizeSink : public LogSink
{
public:
    virtual void log(const char* data,size_t len) override
    {

    }
};// class RollBySizeSink end


class SinkFactory
{
public:
    template<class SinkType,class ...Args>
    static LogSink::ptr create(Args && ...args)
    {
        return std::make_shared<SinkType>(std::forward<Args>(args)...);
    }
};// class SinFactory end

}// namespace nhylog end