#pragma once
#include "format.hpp"
#include "sink.hpp"

#include "looper.hpp"

#include <atomic>
#include <cstdarg>
#include <mutex>
#include <unordered_map>


namespace nhylog
{
enum class LoggerType
{
    LOGGER_SYNC,LOGGER_ASYNC
};

class Logger
{
public:
    using ptr = std::shared_ptr<Logger>;
    Logger(
        const std::string logger_name,
        LogLevel::Level limit_level,
        Formatter::ptr formatter,
        std::vector<LogSink::ptr> sinks
    ):
        logger_name_(logger_name),
        limit_level_(limit_level),
        formatter_(formatter),
        sinks_(sinks){
    }


    void debug(const std::string& file,size_t line,const std::string fmt,...){
        if(limit_level_ > LogLevel::Level::DEBUG) return;
        va_list ap;
        va_start(ap,fmt);
        char* res;
        int n = vasprintf(&res,fmt.c_str(),ap);
        if(n == -1) {std::cout << "vasprintf error" << std::endl;return;}
        va_end(ap);
        serialize(LogLevel::Level::DEBUG,file,line,res);
        free(res);
    }
    void info(const std::string& file,size_t line,const std::string fmt,...){
        if(limit_level_ > LogLevel::Level::INFO) return;
        va_list ap;
        va_start(ap,fmt);
        char* res;
        int n = vasprintf(&res,fmt.c_str(),ap);
        if(n == -1) {std::cout << "vasprintf error" << std::endl;return;}
        va_end(ap);
        serialize(LogLevel::Level::INFO,file,line,res);
        free(res);
    }
    void warn(const std::string& file,size_t line,const std::string fmt,...){
        if(limit_level_ > LogLevel::Level::WARN) return;
        va_list ap;
        va_start(ap,fmt);
        char* res;
        int n = vasprintf(&res,fmt.c_str(),ap);
        if(n == -1) {std::cout << "vasprintf error" << std::endl;return;}
        va_end(ap);
        serialize(LogLevel::Level::WARN,file,line,res);
        free(res);
    }
    void error(const std::string& file,size_t line,const std::string fmt,...){
        if(limit_level_ > LogLevel::Level::ERROR) return;
        va_list ap;
        va_start(ap,fmt);
        char* res;
        int n = vasprintf(&res,fmt.c_str(),ap);
        if(n == -1) {std::cout << "vasprintf error" << std::endl;return;}
        va_end(ap);
        serialize(LogLevel::Level::ERROR,file,line,res);
        free(res);
    }
    void fatal(const std::string& file,size_t line,const std::string fmt,...){
        if(limit_level_ > LogLevel::Level::FATAL) return;
        va_list ap;
        va_start(ap,fmt);
        char* res;
        int n = vasprintf(&res,fmt.c_str(),ap);
        if(n == -1) {std::cout << "vasprintf error" << std::endl;return;}
        va_end(ap);
        serialize(LogLevel::Level::FATAL,file,line,res);
        free(res);
    }

    const std::string& getLoggerName(){
        return logger_name_;
    }
protected:
    void serialize(LogLevel::Level level,const std::string& file,size_t line,const char* res){
        LogMsg msg(level,line,file,logger_name_,res);
        std::stringstream ss; 
        formatter_->format(ss,msg);
        log(ss.str().c_str(),ss.str().size());
    }

    virtual void log(const char* data, size_t len) = 0;
protected:
    std::string logger_name_;
    std::atomic<LogLevel::Level> limit_level_;
    Formatter::ptr formatter_;
    std::vector<LogSink::ptr> sinks_;
    std::mutex mutex_;
};// class Logger end


class SyncLogger : public Logger
{
public:
    SyncLogger(
        const std::string logger_name,
        LogLevel::Level limit_level,
        Formatter::ptr formatter,
        std::vector<LogSink::ptr> sinks
    ):
        Logger(logger_name,limit_level,formatter,sinks)
    {}

    virtual void log(const char* data,size_t len) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(sinks_.empty()) return;
        for(auto& sink : sinks_){
            sink->log(data,len);
        }
    }
};// class SyncLogger end

class AsyncLogger : public Logger
{
public:
    AsyncLogger(
        const std::string logger_name,
        LogLevel::Level limit_level,
        Formatter::ptr formatter,
        std::vector<LogSink::ptr> sinks,
        AsyncType looper_type
    ):
        Logger(logger_name,limit_level,formatter,sinks),
        looper_(std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::realLog,this,std::placeholders::_1),looper_type))
    {}
    

    virtual void log(const char* data, size_t len) override{
        looper_->push(data,len);
    }
    void realLog(Buffer& buf){
        if(sinks_.empty()) return;
        for(auto& sink : sinks_){
            sink->log(buf.begin(),buf.readAbleSize());
        }
    }
private:
    AsyncLooper::ptr looper_;
};// class AsyncLogger end


class LoggerBuilder
{
public:
    LoggerBuilder():
        logger_type_(LoggerType::LOGGER_SYNC),
        limit_level_(LogLevel::Level::DEBUG) ,
        looper_type_(AsyncType::ASYNC_SAFE)
    {}
    void buildLoggerType(LoggerType logger_type){
        logger_type_ = logger_type;
    }
    void buildLoggerName(const std::string& logger_name){
        logger_name_ = logger_name;
    }
    void buildLimitLevel(LogLevel::Level limit_level){
        limit_level_ = limit_level;
    }
    void buildFormatter(const std::string pattern){
        formatter_ = std::make_shared<Formatter>(pattern);
    }
    template<class SinkType, class ...Args>
    void buildSink(Args&& ...args){
        LogSink::ptr sink = SinkFactory::create<SinkType>(std::forward<Args>(args)...);
        sinks_.push_back(sink);
    }   
    void buildLooperType(AsyncType looper_type){
        looper_type_ = looper_type;
    }

    virtual Logger::ptr build() = 0;
protected:
    LoggerType logger_type_;
    std::string logger_name_;
    std::atomic<LogLevel::Level> limit_level_;
    Formatter::ptr formatter_;
    std::vector<LogSink::ptr> sinks_;
    AsyncType looper_type_;
};// class LoggerBuilder end


class LocalLoggerBuilder : public LoggerBuilder
{
public:
    virtual Logger::ptr build(){
        assert(!logger_name_.empty());
        if(formatter_  == nullptr){
            formatter_ = std::make_shared<Formatter>();
        }
        if(sinks_.empty()){
            buildSink<StdoutSink>();
        }
        if(logger_type_ == LoggerType::LOGGER_ASYNC){
            return std::make_shared<AsyncLogger>(logger_name_,limit_level_,formatter_,sinks_,looper_type_);
        }else if(logger_type_ == LoggerType::LOGGER_SYNC){
            return std::make_shared<SyncLogger>(logger_name_,limit_level_,formatter_,sinks_);
        }
        return nullptr;
    }

};// class LocalLoggerBuilder end

class LoggerManager
{
public:
    static LoggerManager& getInstance(){
        static LoggerManager lm;
        return lm;
    }

    bool hasLogger(const std::string& logger_name){
        std::unique_lock<std::mutex> lock(mutex_);
        return !(loggers_.find(logger_name) == loggers_.end());
    }

    void addLogger(Logger::ptr& logger){
        if(hasLogger(logger->getLoggerName())) return;
        std::unique_lock<std::mutex> lock(mutex_);
        loggers_.insert(std::make_pair(logger->getLoggerName(),logger));
    }

    Logger::ptr getLogger(const std::string& logger_name){
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = loggers_.find(logger_name);
        if(it == loggers_.end()) return Logger::ptr();
        else {
            return it->second;
        }
    }

    Logger::ptr getRootLogger(){
        return root_logger_;
    }
private:
    LoggerManager(){
        std::unique_ptr<LocalLoggerBuilder> builder(new LocalLoggerBuilder());
        builder->buildLoggerName("root");
        root_logger_ = builder->build();
        loggers_.insert(std::make_pair("root",root_logger_));
    }
private:
    std::mutex mutex_;
    Logger::ptr root_logger_;
    std::unordered_map<std::string,Logger::ptr> loggers_;
};// class LoggerManager end

class GlobalLoggerBuilder : public LoggerBuilder
{
public:
    virtual Logger::ptr build(){
        assert(!logger_name_.empty());
        if(formatter_  == nullptr){
            formatter_ = std::make_shared<Formatter>();
        }
        if(sinks_.empty()){
            buildSink<StdoutSink>();
        }
        Logger::ptr logger;
        if(logger_type_ == LoggerType::LOGGER_ASYNC){
            logger = std::make_shared<AsyncLogger>(logger_name_,limit_level_,formatter_,sinks_,looper_type_);
        }else if(logger_type_ == LoggerType::LOGGER_SYNC){
            logger = std::make_shared<SyncLogger>(logger_name_,limit_level_,formatter_,sinks_);
        }
        LoggerManager::getInstance().addLogger(logger);
        return logger;
    }
};// class GlobalLoggerBuilder end


}// namespace nhylog end