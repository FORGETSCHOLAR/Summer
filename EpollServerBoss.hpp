#pragma once
#include <memory>
#include "Handlers/Acceptor.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"
#include "Connection.hpp"
#include "Log/nhylog.hpp"
#include "WorkerInfo.hpp"
#include "EpollServerWorker.hpp"

#include <mutex>
#include <condition_variable>
#include <queue>

class EpollServerBoss
{

public:
    EpollServerBoss()
    {
    }

    ~EpollServerBoss()
    {
        epoller_.Close();
    }

    void init(uint16_t port)
    {

        // 1 创建listen套接字，负责接收连接请求
        listenSock_.Socket();
        listenSock_.Bind(port);
        listenSock_.Listen();

        // 2 创建epoll模型
        epoller_.Create();

        // 3 将listen套接字写入底层epoll进行监听
        epoller_.AddModEvent(listenSock_.Fd(), EPOLLIN | EPOLLET, EPOLL_CTL_ADD);
    }

    /**
     * @brief 面试Worker，将其WorkerInfo保存起来
     *
     */
    void interview(EpollServerWorker &worker)
    {
        worker_infos_.emplace_back(
            worker.getEpoller(),
            worker.getConnections(),
            worker.receiver_,
            worker.sender_,
            worker.excepter_);
    }

    // 设置为虚函数，继承自EpollServerBoss
    virtual void start(int timeout)
    {
        while (true)
        {
            loopOnce(timeout);
        }
    }

private:
    void loopOnce(int timeout)
    {
        int n = epoller_.Wait(revs_, 128, timeout);
        for (int i = 0; i < n; i++)
        {
            int fd = revs_[i].data.fd;
            uint32_t events = revs_[i].events;
            if ((events & EPOLLHUP) || (events & EPOLLERR))
                events |= (EPOLLIN);
            if ((events & EPOLLIN))
            {
                std::string clientIp;
                uint16_t clientPort;
                int sock = listenSock_.Accept(&clientIp, &clientPort);
                logger->info("Acceptor fd %d, clientIp:%s clientPort:%d", sock, clientIp.c_str(), clientPort);

                // 将sock保存到共享队列中
                // 真正做到责任分离和确认，Boss就是负责处理连接的,后面要做成负载均衡的
                AddConnection(sock, EPOLLIN | EPOLLET, worker_infos_[step]);

                step += 1;
                step %= worker_infos_.size();
            }
            else
            {
                logger->info("出现了不应该被Boss关注的事件 %d", events);
            }
        }
    }

    void AddConnection(int fd, uint32_t events,
                       WorkerInfo &workinfo,
                       const std::string &ip = "127.0.0.1",
                       const uint16_t &port = 8080)
    {

        // 1 先判断是不是ET模式，如果是ET，则要设置为非阻塞
        if (events & EPOLLET)
        {
            FileUtils::setNonBlock(fd);
        }

        // 2 创建connection信息
        Connection *conn = new Connection(fd, ip, port);

        // 绑定方法
        conn->Register(workinfo.receiver_, workinfo.sender_, workinfo.excepter_);

        // 记录连接关注的事件
        conn->events_ = events;

        workinfo.connections_.insert({fd, conn});

        // 3 写入内核
        workinfo.epoller_.AddModEvent(fd, conn->events_, EPOLL_CTL_ADD);
    }

private:
    Sock listenSock_;
    Epoller epoller_;
    epoll_event revs_[128];

    std::vector<WorkerInfo> worker_infos_;
    int step = 0;

private:
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};