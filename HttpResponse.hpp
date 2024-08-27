#pragma once
/**
 * 封装响应
 */
#include <string>
#include <unordered_map>

class HttpResponse
{
public:
    HttpResponse() = default;

    // 设置状态码和状态描述
    void setStatus(int code, const std::string &description)
    {
        statusCode = code;
        statusDescription = description;
    }

    // 添加响应头
    void addHeader(const std::string &key, const std::string &value)
    {
        headers[key] = value;
    }

    // 设置响应体
    void setBody(const std::string &bodyContent)
    {
        body = bodyContent;
        addHeader("Content-Length", std::to_string(body.size()));
    }

    // 序列化为HTTP响应字符串
    std::string toString() const
    {
        std::string response;

        // 添加状态行
        response += "HTTP/1.1 " + std::to_string(statusCode) + " " + statusDescription + "\r\n";

        // 添加响应头
        for (const auto &header : headers)
        {
            response += header.first + ": " + header.second + "\r\n";
        }

        // 添加空行
        response += "\r\n";

        // 添加响应体
        response += body;

        return response;
    }

private:
    int statusCode = 200;                       // 默认状态码200 OK
    std::string statusDescription = "OK";       // 默认状态描述
    std::unordered_map<std::string, std::string> headers;  // 响应头
    std::string body;                           // 响应体
};


/*
HttpResponse response;
response.setStatus(200, "OK");
response.addHeader("Content-Type", "text/plain");
response.setBody("Hello, World!");

std::string responseString = response.toString();
*/