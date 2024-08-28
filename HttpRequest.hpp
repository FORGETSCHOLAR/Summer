#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <algorithm>

class HttpRequest
{
public:
    HttpRequest() = default;

    // 解析HTTP请求
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

        // 移动到空行（头部与体之间的空行）之后
        pos = line_end + 2;
        if (pos < request.size())
        {
            body = request.substr(pos); // 提取请求体
        }

        return true;
    }

    static bool extractCompleteRequest(std::string &inbuffer, std::string &completeRequest)
    {
        size_t pos = inbuffer.find("\r\n\r\n");
        if (pos == std::string::npos)
        {
            return false; // 未找到完整的请求
        }

        // 提取完整的请求
        size_t contentLength = 0;
        std::string headers = inbuffer.substr(0, pos + 4); // 包括请求头和空行

        // 查找 Content-Length 头并确定请求体的长度
        size_t contentLengthPos = headers.find("Content-Length:");
        if (contentLengthPos != std::string::npos)
        {
            contentLength = std::stoul(headers.substr(contentLengthPos + 15));
        }

        // 检查请求是否足够长以包含整个请求体
        if (inbuffer.size() < pos + 4 + contentLength)
        {
            return false; // 请求体未完全接收
        }

        completeRequest = inbuffer.substr(0, pos + 4 + contentLength); // 包括请求头和请求体
        inbuffer.erase(0, pos + 4 + contentLength);                    // 从缓冲区中移除已处理的部分
        return true;
    }

    // 获取 Cookie
    std::string getCookie(const std::string &name)
    {
        if (cookieMap.empty())
        {
            parseCookies();
        }
        auto it = cookieMap.find(name);
        return it != cookieMap.end() ? it->second : "";
    }

    // 设置查询参数
    void setQueryParametersMultiple(std::map<std::string, std::vector<std::string>>& queryParameterss)
    {
        queryParameters = queryParameterss;
    }

    // 获取查询参数
    std::vector<std::string> getQueryParametersMultiple(const std::string &name)
    {
        if (queryParameters.empty())
        {
            parseQueryParameters();
        }
        auto it = queryParameters.find(name);
        return it != queryParameters.end() ? it->second : std::vector<std::string>();
    }

    // 获取POST参数
    std::vector<std::string> getPostParametersMultiple(const std::string &name)
    {
        if (isFormPost())
        {
            if (postParameters.empty())
            {
                parsePostParameters();
            }
            auto it = postParameters.find(name);
            return it != postParameters.end() ? it->second : std::vector<std::string>();
        }
        else if (isJsonPost())
        {
        }
        return {};
    }

    // 判断body是否为form表单
    bool isFormPost() const
    {
        auto it = headers.find("content-type");
        if (it != headers.end())
        {
            std::string contentType = it->second;
            std::transform(contentType.begin(), contentType.end(), contentType.begin(), ::tolower);
            return method == "POST" &&
                   (contentType.find("multipart/form-data") != std::string::npos ||
                    contentType.find("application/x-www-form-urlencoded") != std::string::npos);
        }
        return false;
    }

    // 判断body是否为json
    bool isJsonPost() const
    {
        return method == "POST" && headers.at("Content-Type").find("application/json") != std::string::npos;
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

            trim(key);
            trim(value);

            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c)
                           { return std::tolower(c); });

            headers[key] = value;
        }
    }

    // 解析 Cookie
    void parseCookies()
    {
        auto it = headers.find("cookie");
        if (it != headers.end())
        {
            std::string cookieStr = it->second;
            size_t pos = 0;
            while ((pos = cookieStr.find("; ")) != std::string::npos)
            {
                std::string cookie = cookieStr.substr(0, pos);
                size_t eq_pos = cookie.find('=');
                if (eq_pos != std::string::npos)
                {
                    cookieMap[cookie.substr(0, eq_pos)] = cookie.substr(eq_pos + 1);
                }
                cookieStr.erase(0, pos + 2);
            }
            // 最后一个cookie
            size_t eq_pos = cookieStr.find('=');
            if (eq_pos != std::string::npos)
            {
                cookieMap[cookieStr.substr(0, eq_pos)] = cookieStr.substr(eq_pos + 1);
            }
        }
    }

    // 解析查询参数
    void parseQueryParameters()
    {
        size_t query_start = uri.find('?');
        if (query_start != std::string::npos)
        {
            std::string query_str = uri.substr(query_start + 1);
            size_t pos = 0;
            while ((pos = query_str.find('&')) != std::string::npos)
            {
                std::string param = query_str.substr(0, pos);
                addParameter(param, queryParameters);
                query_str.erase(0, pos + 1);
            }
            // 最后一个参数
            addParameter(query_str, queryParameters);
        }
    }

    // 解析POST参数
    void parsePostParameters()
    {
        std::string body_str = body;
        size_t pos = 0;
        while ((pos = body_str.find('&')) != std::string::npos)
        {
            std::string param = body_str.substr(0, pos);
            addParameter(param, postParameters);
            body_str.erase(0, pos + 1);
        }
        // 最后一个参数
        addParameter(body_str, postParameters);
    }

    // 辅助函数：解析单个参数并加入到 map 中
    void addParameter(const std::string &param, std::map<std::string, std::vector<std::string>> &paramMap)
    {
        size_t eq_pos = param.find('=');
        if (eq_pos != std::string::npos)
        {
            std::string key = param.substr(0, eq_pos);
            std::string value = param.substr(eq_pos + 1);
            paramMap[key].push_back(value);
        }
    }

    void trim(std::string &str)
    {
        str.erase(0, str.find_first_not_of(' '));
        str.erase(str.find_last_not_of(' ') + 1);
    }

public:
    std::string &getUri()
    {
        return uri;
    }

    std::string& getMethod(){
        return method;
    }

    std::string& getBody(){
        return body;
    }

private:
    std::string method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::map<std::string, std::string> cookieMap;
    std::map<std::string, std::vector<std::string>> queryParameters;
    std::map<std::string, std::vector<std::string>> postParameters;
    std::string body;

    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};
