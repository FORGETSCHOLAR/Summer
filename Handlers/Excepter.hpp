#pragma once
#include "IHandler.hpp"

class Excepter : public IEventHandler
{
public:
    Excepter(Epoller &epoller,std::unordered_map<int, Connection *> &connections)
        :epoller_(epoller),connections_(connections)
    {

    }

    void handle(Connection *conn) override
    {

        int fd = conn->fd_;
        logger->info("Exceptor::正在断开一个请求 %d", fd);
        epoller_.DelEvent(conn->fd_);
        connections_.erase(conn->fd_);
        close(conn->fd_);
        delete conn;
        logger->info("Exceptor::成功断开一个请求 fd = %d", fd);
    }
private:
    Epoller &epoller_;
    std::unordered_map<int, Connection *> &connections_;
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};
