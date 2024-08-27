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
    EventLoop(std::shared_ptr<IEventHandler> receiver,
              std::shared_ptr<IEventHandler> sender,
              std::shared_ptr<IEventHandler> excepter,
              std::shared_ptr<IEventHandler> acceptor,
              Epoller &epoller,
              Sock &listenSock,
              std::unordered_map<int, Connection *> &connections,
              epoll_event *revs)
        : receiver_(receiver),
          sender_(sender),
          excepter_(excepter),
          acceptor_(acceptor),
          epoller_(epoller),
          listenSock_(listenSock),
          connections_(connections),
          revs_(revs) {}

    void loopOnce(int timeout)
    {
        logger->info("EventLoop::LoopOnce");
        int n = epoller_.Wait(revs_, 128, timeout);
        logger->info("LoopOnce::%d", n);

        for (int i = 0; i < n; i++)
        {
            int fd = revs_[i].data.fd;
            uint32_t events = revs_[i].events;
            if ((events & EPOLLHUP) || (events & EPOLLERR))
                events |= (EPOLLIN | EPOLLOUT);
            if ((events & EPOLLIN) && ConnIsExist(fd)){
                logger->info("fd %d 读事件就绪", fd);
                connections_[fd]->handleRead();
            }
            if ((events & EPOLLOUT) && ConnIsExist(fd)){
                logger->info("fd %d 写事件就绪", fd);
                connections_[fd]->handleWrite();
            }
        }
    }

    void loop(int timeout){
        while(true){
            loopOnce(timeout);
        }
    }

private:
    bool ConnIsExist(int fd)
    {
        return connections_.find(fd) != connections_.end();
    }

private:
    std::shared_ptr<IEventHandler> receiver_;
    std::shared_ptr<IEventHandler> sender_;
    std::shared_ptr<IEventHandler> excepter_;
    std::shared_ptr<IEventHandler> acceptor_;
    Epoller &epoller_;
    Sock &listenSock_;
    std::unordered_map<int, Connection *> &connections_;
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
    epoll_event *revs_;
};
