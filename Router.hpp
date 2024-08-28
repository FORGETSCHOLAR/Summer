#pragma once

#include <string>
#include <regex>
#include <functional>
#include <unordered_map>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Utils/PathUtils.hpp"

class Route
{
public:
    Route(const std::string &pattern,
          std::function<void(HttpRequest &, HttpResponse &)> handler)
        : pattern_(pattern), handler_(handler) {}

    void handle(HttpRequest &request, HttpResponse &response) const
    {
        handler_(request, response);
    }


public:
    std::string pattern_;
    std::function<void(HttpRequest &, HttpResponse &)> handler_;
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
        for (int i = 0; i < routes_.size();i++)
        {
            const Route& route = routes_[i];
            //
            std::map<std::string, std::vector<std::string>> queryParameters;
            if (AntMatcher::match(route.pattern_, request.getUri(),queryParameters))
            {
                request.setQueryParametersMultiple(queryParameters);
                route.handle(request, response);
                return true;
            }
        }
        
        return false; // 如果没有匹配，返回false
    }

private:
    std::vector<Route> routes_;
};
