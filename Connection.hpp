#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

#include "Epoller.hpp"
#include "Socket.hpp"
#include "Utils/FileUtils.hpp"
#include "Exception/Exception.hpp"
#include "Handlers/IHandler.hpp"
#include "Log/nhylog.hpp"

class Connection
{
public:
    using callback_t = std::function<void(Connection *)>;

    Connection(const int &fd, const std::string &ip, const uint16_t &port)
        : fd_(fd), ip_(ip), port_(port)
    {
    }

    void Register(std::shared_ptr<IEventHandler> receiver, 
                  std::shared_ptr<IEventHandler> sender, 
                  std::shared_ptr<IEventHandler> excepter)
    {
        receiver_ = receiver;
        sender_ = sender;
        excepter_ = excepter;
    }

    void handleRead() {
        logger->debug("receiver 为空");
        if (receiver_) {
            logger->debug("receiver 不为空");
            receiver_->handle(this);
        }
    }

    void handleWrite() {
        if (sender_) {
            sender_->handle(this);
        }
    }

    void handleException() {
        if (excepter_) {
            excepter_->handle(this);
        }
    }

public:
    // IO信息
    int fd_;
    std::string inbuffer_;
    std::string outbuffer_;

    // User信息
    std::string ip_;
    uint16_t port_;

    // IO处理函数
    std::shared_ptr<IEventHandler> receiver_;
    std::shared_ptr<IEventHandler> sender_;
    std::shared_ptr<IEventHandler> excepter_;

    // 自己要关心的事件
    uint32_t events_;

    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};
