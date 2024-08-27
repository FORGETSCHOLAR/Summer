#pragma once

#include <string>
#include <regex>
#include <functional>
#include <unordered_map>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"


class Route
{
public:
    Route(const std::string &pattern,
          std::function<void(HttpRequest &, HttpResponse &)> handler)
        : pattern_(pattern), handler_(handler), regexPattern_(convertToRegex(pattern)) {}

    bool matches(const std::string &path, std::unordered_map<std::string, std::string> &params) const
    {
        std::smatch match;
        if (std::regex_match(path, match, regexPattern_))
        {
            extractParams(match, params);
            return true;
        }
        return false;
    }

    void handle(HttpRequest &request, HttpResponse &response) const
    {
        handler_(request, response);
    }
private:
    std::regex convertToRegex(const std::string &pattern) const
    {
        std::string regexStr = "^" + pattern + "$";
        regexStr = std::regex_replace(regexStr, std::regex("\\{([^\\}:]+)\\}"), "(?<$1>[^/]+)");
        regexStr = std::regex_replace(regexStr, std::regex("\\{([^\\}:]+):([^\\}]+)\\}"), "(?<$1>$2)");
        return std::regex(regexStr);
    }

    void extractParams(const std::smatch &match, std::unordered_map<std::string, std::string> &params) const
    {
        for (size_t i = 1; i < match.size(); ++i)
        {
            params[match[i].str()] = match.str(i);
        }
    }
private:
    std::string pattern_;
    std::function<void(HttpRequest &, HttpResponse &)> handler_;
    std::regex regexPattern_;

};



class Router
{
public:
    void addRoute(const std::string &pattern, std::function<void(HttpRequest &, HttpResponse &)> handler)
    {
        routes_.emplace_back(pattern, handler);
    }

    bool route(HttpRequest &request, HttpResponse &response) const
    {
        std::unordered_map<std::string, std::string> params;
        for (const auto &route : routes_)
        {
            if (route.matches(request.getUri(), params))
            {
                // 在这里将参数注入到request对象中
                // request.setParams(params);
                route.handle(request, response);
                return true;
            }
        }
        
        return false; // 如果没有匹配，返回false
    }

private:
    std::vector<Route> routes_;
};
