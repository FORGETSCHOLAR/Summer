#pragma once
#include <iostream>
#include <memory>
#include <ctime>
#include <vector>
#include <cassert>
#include <sstream>

#include "level.hpp"
#include "message.hpp"

namespace nhylog
{

class FormatItem
{
public:
    using ptr = std::shared_ptr<FormatItem>;
    virtual void format(std::ostream& out,LogMsg& msg) = 0;
};


class LevelFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << LogLevel::toString(msg.level_);
    }
};

class TimeFormatItem : public FormatItem
{
public:
    TimeFormatItem(const std::string& fmt = "%H%M%S"):fmt_(fmt){}
    virtual void format(std::ostream& out,LogMsg& msg){
        struct tm t;
        localtime_r(&msg.ctime_,&t);
        char tmp[32] = {0};
        strftime(tmp,31,fmt_.c_str(),&t);
        out << tmp;
    }
private:
    std::string fmt_;
};

class LineFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << msg.line_;
    }
};

class ThreadFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << msg.tid_;
    }
};

class FileFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << msg.file_;
    }
};

class LoggerFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << msg.logger_;
    }
};

class MsgFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << msg.payload_;
    }
};

class NFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << "\n";
    }
};

class TableFormatItem : public FormatItem
{
public:
    virtual void format(std::ostream& out,LogMsg& msg){
        out << "\t";
    }
};

class OtherFormatItem : public FormatItem
{
public:
    OtherFormatItem(const std::string& str):str_(str){}
    virtual void format(std::ostream& out,LogMsg& msg){
        out << str_;
    }
private:
    std::string str_;
};




class Formatter
{
public:
    using ptr = std::shared_ptr<Formatter>;
    Formatter(const std::string& pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n")
    :pattern_(pattern)
    {assert(parsePattern());}

    std::string format(LogMsg& msg){
        std::stringstream ss;
        format(ss,msg);
        return ss.str();
    }

    void format(std::ostream& out,LogMsg& msg)
    {
        for(auto& item : items_){
            item->format(out,msg);
        }
    }
private:
    FormatItem::ptr createItem(const std::string& key,const std::string& val)
    {
        if(key == "p") return std::make_shared<LevelFormatItem>();
        if(key == "d") return std::make_shared<TimeFormatItem>(val);
        if(key == "l") return std::make_shared<LineFormatItem>();
        if(key == "t") return std::make_shared<ThreadFormatItem>();
        if(key == "f") return std::make_shared<FileFormatItem>();
        if(key == "c") return std::make_shared<LoggerFormatItem>();
        if(key == "m") return std::make_shared<MsgFormatItem>();
        if(key == "n") return std::make_shared<NFormatItem>();
        if(key == "T") return std::make_shared<TableFormatItem>();
        if(key == "")  return std::make_shared<OtherFormatItem>(val);

        std::cout << "没有该格式化字符：%"<< key <<std::endl;
        abort();
    }

    bool parsePattern()
    {
        std::vector<std::pair<std::string, std::string>> fmt_order;
        size_t pos = 0;
        std::string key, val;
        while (pos < pattern_.size())
        {

            if (pattern_[pos] != '%')
            {
                val.push_back(pattern_[pos++]);
                continue;
            }

            // 走到这判断是否是一个普通的%
            if (pos + 1 < pattern_.size() && pattern_[pos + 1] == '%')
            {
                val.push_back('%');
                pos += 2;
                continue;
            }

            // 能到这里说明%后面是个格式化字符，说明原始字符串处理完毕了
            if (!val.empty())
            {
                fmt_order.push_back(std::make_pair("", val));
                val.clear();
            }

            // 现在这是%，走一步就是key，格式化字符的处理
            pos += 1;
            if (pos == pattern_.size())
            {
                std::cout << "% 后没有格式化字符" << std::endl;
                return false;
            }
            key = pattern_[pos];

            // pos指向格式化字符后的位置，判断是否为字串
            pos += 1;
            // 防止最后一个
            if (pos < pattern_.size() && pattern_[pos] == '{')
            {
                pos += 1;
                while (pos < pattern_.size() && pattern_[pos] != '}')
                {
                    val.push_back(pattern_[pos++]);
                }
                // 走到末尾，还没找到花括号，出错了，格式错误
                if (pos == pattern_.size())
                {
                    std::cout << "花括号匹配出错" << std::endl;
                    return false;
                }
                pos += 1; // pos指向},走一步去新位置
            }

            fmt_order.push_back(std::make_pair(key, val));
            key.clear();
            val.clear();
        }

        // 2 根据解析得到的数据格式化子项数组成员
        for (auto &item : fmt_order)
        {
            items_.push_back(createItem(item.first, item.second));
        }

        return true;
    }


        
private:
    std::vector<FormatItem::ptr> items_;
    std::string pattern_;
};// class Formatter end

}// nhylog end