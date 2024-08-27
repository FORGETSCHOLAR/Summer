#pragma once
#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

namespace nhylog
{
namespace util
{
class File
{
public:
    static bool exists(const std::string& pathname)
    {
        struct stat st;
        if(stat(pathname.c_str(),&st) < 0) return false;
        else return true;
    }

    static std::string path(const std::string& pathname)
    {
        size_t pos = pathname.find_last_of("/\\");
        if(pos == std::string::npos) return ".";
        else return pathname.substr(0,pos+1);
    }

    static bool createDir(const std::string& pathname)
    {
        size_t idx = 0,pos = 0;
        while(idx < pathname.size())
        {
            pos = pathname.find_first_of("/\\");
            if(std::string::npos == pos) mkdir(pathname.c_str(),0777);
            std::string parent_dir = pathname.substr(0,pos+1);
            if(exists(parent_dir)){
                idx = pos+1;
            }else {
                mkdir(parent_dir.c_str(), 0777);
                idx = pos+1;
            }
        }

        return true;
    }
};//class File end



class Date
{
public:
    static time_t now()
    {
        return (time_t)time(nullptr);
    }
};//class Date end



}//nhylog::util end
}//nhylog end