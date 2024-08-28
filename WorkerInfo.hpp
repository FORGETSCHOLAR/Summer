#pragma once

#include "Epoller.hpp"
#include "Connection.hpp"

class WorkerInfo
{
public:
    WorkerInfo(Epoller &epoller,
               std::unordered_map<int, Connection *> &connections, std::shared_ptr<IEventHandler> receiver,
               std::shared_ptr<IEventHandler> sender,
               std::shared_ptr<IEventHandler> excepter) 
               : epoller_(epoller), 
               connections_(connections),
               receiver_(receiver),
               sender_(sender),
               excepter_(excepter)
    {
    }

    Epoller &epoller_;
    std::unordered_map<int, Connection *> &connections_;

    std::shared_ptr<IEventHandler> receiver_;
    std::shared_ptr<IEventHandler> sender_;
    std::shared_ptr<IEventHandler> excepter_;
};