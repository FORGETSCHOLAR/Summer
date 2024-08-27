#pragma once 

/**
 * CopyRight (C) , 2023-2033 ,GOSUN CL
 * 
 * @file nhylog.hpp
 * @brief 整合日志器功能、简化用户操作
 * 
 * @author nhy
 * @version v1.0.0 20240327
*/

#include "logger.hpp"
namespace nhylog
{
Logger::ptr getLogger(const std::string& logger_name){
    return LoggerManager::getInstance().getLogger(logger_name);
}


Logger::ptr getRootLogger(){
    return LoggerManager::getInstance().getRootLogger();
}

#define debug(fmt,...) debug(__FILE__,__LINE__,fmt,##__VA_ARGS__) 
#define info(fmt,...) info(__FILE__,__LINE__,fmt,##__VA_ARGS__) 
#define warn(fmt,...) warn(__FILE__,__LINE__,fmt,##__VA_ARGS__) 
#define error(fmt,...) error(__FILE__,__LINE__,fmt,##__VA_ARGS__) 
#define fatal(fmt,...) fatal(__FILE__,__LINE__,fmt,##__VA_ARGS__) 

#define DEBUG(fmt,...) nhylog::getRootLogger()->debug(fmt,##__VA_ARGS__)
#define INFO(fmt,...) nhylog::getRootLogger()->info(fmt,##__VA_ARGS__)
#define WARN(fmt,...) nhylog::getRootLogger()->warn(fmt,##__VA_ARGS__)
#define ERROR(fmt,...) nhylog::getRootLogger()->error(fmt,##__VA_ARGS__)
#define FATAL(fmt,...) nhylog::getRootLogger()->fatal(fmt,##__VA_ARGS__)

}//namespace nhylog end


