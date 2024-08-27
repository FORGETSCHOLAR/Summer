#include <iostream>
#include "logger.hpp"
#include "nhylog.hpp"

using namespace std;

void test_LocalLoggerBuilder(){
    nhylog::LocalLoggerBuilder builder;
    builder.buildLoggerName("sync_logger");
    builder.buildLoggerType(nhylog::LoggerType::LOGGER_SYNC);
    builder.buildLimitLevel(nhylog::LogLevel::Level::DEBUG);
    auto logger = builder.build();

    for(int i = 0;i<10;i++){
        logger->debug("this is a test log : %d",i);
        sleep(1);
    }
}

void test_GlobalLoggerBuilder(){
    std::unique_ptr<nhylog::LoggerBuilder> builder(new nhylog::GlobalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildLoggerType(nhylog::LoggerType::LOGGER_ASYNC);
    builder->buildLooperType(nhylog::AsyncType::ASYNC_SAFE);
    builder->buildLimitLevel(nhylog::LogLevel::Level::DEBUG);  
    builder->buildSink<nhylog::FileSink>("./log.message");
    builder->build();

    auto logger = nhylog::LoggerManager::getInstance().getLogger("async_logger");
    for(int i = 0;i<10;i++){
        logger->debug("this is a test log : %d",i);
        logger->debug("helloworld");
        sleep(1);
    }
}


void test_rootLogger(){
    DEBUG("this is a test log %s", "do you like it?");
    nhylog::getRootLogger()->fatal("error!");
}

int main()
{   

    test_rootLogger();
    // test_LocalLoggerBuilder();
    // test_GlobalLoggerBuilder();
    return 0;
}