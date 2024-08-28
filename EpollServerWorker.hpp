#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>

#include "Connection.hpp"
#include "Handlers/Receiver.hpp"
#include "Handlers/Sender.hpp"
#include "Handlers/Excepter.hpp"
#include "Router.hpp"
#include "EventLoop.hpp"
#include "Epoller.hpp"

class EpollServerWorker
{
public:
    EpollServerWorker(Router &router) : router_(router)
    {
    }

    ~EpollServerWorker(){
        epoller_.Close();
    }

    void init()
    {
        // 1、创建Epoll模型
        epoller_.Create();

        eventloop_ = std::make_shared<EventLoop>(epoller_, connections_, revs_);

        //3
        receiver_ = std::make_shared<Receiver>(router_);
        sender_ = std::make_shared<Sender>(epoller_);
        excepter_ = std::make_shared<Excepter>(epoller_, connections_);
    }

    void start(int timeout)
    {
        while (true)
        {
            eventloop_->loopOnce(timeout);
        }
    }

public:
    Epoller &getEpoller()
    {
        return epoller_;
    }

    std::unordered_map<int, Connection *> & getConnections()
    {
        return connections_;
    }

    //TODO后面再封装起来
    std::shared_ptr<IEventHandler> receiver_;
    std::shared_ptr<IEventHandler> sender_;
    std::shared_ptr<IEventHandler> excepter_;

private:
    Epoller epoller_;
    epoll_event revs_[128];
    std::unordered_map<int, Connection *> connections_;
    Router &router_;
    std::shared_ptr<EventLoop> eventloop_;



private:
    std::shared_ptr<std::queue<int>> connectionQueue_;
    std::shared_ptr<std::mutex> queueMutex_;
    std::shared_ptr<std::condition_variable> cv_;
};