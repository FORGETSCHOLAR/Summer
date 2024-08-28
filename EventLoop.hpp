#pragma once
#include <memory>
#include "Epoller.hpp"
#include "Socket.hpp"
#include "Connection.hpp"
#include "Handlers/IHandler.hpp"

// 后面考虑和EpollServer的合并起来

class EventLoop
{
public:
    EventLoop(
        Epoller &epoller,
        std::unordered_map<int, Connection *> &connections,
        epoll_event *revs)
        : epoller_(epoller),
          connections_(connections),
          revs_(revs) {}

    void loopOnce(int timeout)
    {
        int n = epoller_.Wait(revs_, 128, timeout);
        // logger->info("LoopOnce::%d", n);

        for (int i = 0; i < n; i++)
        {
            int fd = revs_[i].data.fd;
            uint32_t events = revs_[i].events;
            if ((events & EPOLLHUP) || (events & EPOLLERR))
                events |= (EPOLLIN | EPOLLOUT);
            if ((events & EPOLLIN) && ConnIsExist(fd))
            {
                connections_[fd]->handleRead();
            }
            if ((events & EPOLLOUT) && ConnIsExist(fd))
            {
                connections_[fd]->handleWrite();
            }
        }
    }

    void loop(int timeout)
    {
        while (true)
        {
            loopOnce(timeout);
        }
    }

private:
    bool ConnIsExist(int fd)
    {
        return connections_.find(fd) != connections_.end();
    }

private:
    Epoller &epoller_;
    std::unordered_map<int, Connection *> &connections_;
    epoll_event *revs_;

private:
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};
