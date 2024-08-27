#pragma once

#include <string>
#include <memory>

#include "../Log/nhylog.hpp"
#include "../Utils/FileUtils.hpp"
#include "../Connection.hpp"
#include "../Socket.hpp"
#include "IHandler.hpp"

/**
 * impl
 */

class Acceptor : public IEventHandler
{
public:
    Acceptor(Sock &listenSock,
             std::unordered_map<int, Connection *> &connections,
             Epoller &epoller,
             std::shared_ptr<IEventHandler> receiver,
             std::shared_ptr<IEventHandler> sender,
             std::shared_ptr<IEventHandler> excepter)
        : listenSock_(listenSock),
          epoller_(epoller),
          connections_(connections),
          receiver_(receiver),
          sender_(sender),
          excepter_(excepter)
    {
    }

    void handle(Connection *conn) override
    {
        do
        {
            try
            {
                std::string clientIp;
                uint16_t clientPort;
                int sock = listenSock_.Accept(&clientIp, &clientPort);
                logger->info("Acceptor fd %d, clientIp:%s clientPort:%d",sock, clientIp.c_str(), clientPort);
                AddConnection(sock, EPOLLIN | EPOLLET);
            }
            catch (const AcceptException &e)
            {
                int errCode = e.getErrorCode();
                if (errCode == EAGAIN || errCode == EWOULDBLOCK)
                {
                    break;
                }
                else if (errCode == EINTR)
                {
                    logger->error("Acceptor发生错误: %d %s", errCode, e.getErrorMsg().c_str());
                    continue;
                }
                else
                {
                    logger->error("Acceptor发生未知错误: %d %s", errCode, e.getErrorMsg().c_str());
                    continue;
                }
            }

        } while (conn->events_ & EPOLLET);
    }

private:
    void AddConnection(int fd, uint32_t events, const std::string &ip = "127.0.0.1", const uint16_t &port = 8080)
    {

        // 1 先判断是不是ET模式，如果是ET，则要设置为非阻塞
        if (events & EPOLLET)
        {
            FileUtils::setNonBlock(fd);
        }

        // 2 创建connection信息
        Connection *conn = new Connection(fd, ip, port);

        // 绑定方法
        conn->Register(receiver_,sender_,excepter_);

        // 记录连接关注的事件
        conn->events_ = events;

        connections_.insert({fd, conn});

        // 3 写入内核
        epoller_.AddModEvent(fd, conn->events_, EPOLL_CTL_ADD);
    }

private:
    Sock &listenSock_;
    Epoller &epoller_;
    std::unordered_map<int, Connection *> &connections_;
    std::shared_ptr<IEventHandler> receiver_;
    std::shared_ptr<IEventHandler> sender_;
    std::shared_ptr<IEventHandler> excepter_;
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};
