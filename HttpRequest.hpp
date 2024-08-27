#pragma once
/**
 * 封装请求
 */

#include <string>
#include <unordered_map>

class HttpRequest
{
public:
    HttpRequest() = default;

    // 解析HTTP请求
    bool parse(const std::string &request)
    {
        size_t pos = 0;
        size_t line_end = request.find("\r\n", pos);

        if (!parseRequestLine(request.substr(pos, line_end - pos)))
        {
            return false;
        }

        pos = line_end + 2;
        while ((line_end = request.find("\r\n", pos)) != std::string::npos && line_end != pos)
        {
            parseHeaderLine(request.substr(pos, line_end - pos));
            pos = line_end + 2;
        }

        pos = line_end + 2;
        if (pos < request.size())
        {
            body = request.substr(pos);
        }
        return true;
    }
public:
    // 从inbuffer中提取完整的HTTP请求
    static bool extractCompleteRequest(std::string &inbuffer, std::string &completeRequest)
    {
        size_t pos = inbuffer.find("\r\n\r\n");
        if (pos == std::string::npos)
        {
            return false; // 未找到完整的请求
        }

        completeRequest = inbuffer.substr(0, pos + 4); // 包括请求头和空行
        inbuffer.erase(0, pos + 4);                    // 从缓冲区中移除已处理的部分
        return true;
    }

private:
    // 解析HTTP头部
    bool parseRequestLine(const std::string &line)
    {
        size_t method_end = line.find(' ');
        if (method_end == std::string::npos)
            return false;
        method = line.substr(0, method_end);

        size_t uri_end = line.find(' ', method_end + 1);
        if (uri_end == std::string::npos)
            return false;
        uri = line.substr(method_end + 1, uri_end - method_end - 1);

        version = line.substr(uri_end + 1);
        return true;
    }

    // 解析请求头
    void parseHeaderLine(const std::string &line)
    {
        size_t delimiter = line.find(':');
        if (delimiter != std::string::npos)
        {
            std::string key = line.substr(0, delimiter);
            std::string value = line.substr(delimiter + 1);
            headers[key] = value;
        }
    }
public:
    /*封装获得成员的方法*/
    std::string& getUri(){
        return uri;
    }
private:
    std::string method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

