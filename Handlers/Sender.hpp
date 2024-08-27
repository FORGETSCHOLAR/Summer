#pragma once

#include "IHandler.hpp"

class Sender : public IEventHandler
{
public:
    Sender(Epoller &epoller):epoller_(epoller){

    }

    void handle(Connection *conn) override
    {

        do
        {
            int size = send(conn->fd_, conn->outbuffer_.c_str(), conn->outbuffer_.size(), 0);
            if (size > 0)
            {
                conn->outbuffer_.erase(0, size);
                if (conn->outbuffer_.empty())
                {
                    logger->info("Sender::消息已全部发完");
                    break;
                }
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 数据暂时无法发送
                    logger->info("Sender::数据暂时无法发送");
                    break;
                }
                else if (errno == EINTR)
                {
                    logger->info("系统调用被中断, 稍后重试 %d %s", errno, strerror(errno));
                    continue;
                }
                else
                {
                    logger->error("发生异常，关闭连接");
                    conn->handleException();
                    return;
                }
            }

        } while (conn->events_ & EPOLLET);

        if (!conn->outbuffer_.empty())
        {
            epoller_.AddModEvent(conn->fd_, EPOLLIN | EPOLLOUT | EPOLLET, EPOLL_CTL_MOD);
        }
        else
        {
            epoller_.AddModEvent(conn->fd_, EPOLLIN | EPOLLET, EPOLL_CTL_MOD);
        }
    }

private:
    Epoller &epoller_;
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};