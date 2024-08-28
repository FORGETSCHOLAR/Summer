#pragma once
#include <unordered_map>
#include <memory>

#include "Connection.hpp"
#include "Epoller.hpp"
#include "Socket.hpp"
#include "Utils/FileUtils.hpp"
#include "Log/nhylog.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Router.hpp"
#include "Handlers/Acceptor.hpp"
#include "Handlers/Receiver.hpp"
#include "Handlers/Sender.hpp"
#include "Handlers/Excepter.hpp"
#include "EventLoop.hpp"

const int DEFAULT_PORT = 8080;
const int DEFAULT_BUFF_SIZE = 1024;
const int GNUM = 128;

class EpollServer
{
public:
    EpollServer(int port = DEFAULT_PORT) : port_(port)
    {
    }
    ~EpollServer()
    {
        listenSock_.~Sock();
        epoller_.Close();
    }

    void initServer()
    {
        listenSock_.Socket();
        listenSock_.Bind(port_);
        listenSock_.Listen();

        epoller_.Create();

        
        receiver_ = std::make_shared<Receiver>(router_);
        sender_ = std::make_shared<Sender>(epoller_);
        excepter_ = std::make_shared<Excepter>(epoller_, connections_);
        acceptor_ = std::make_shared<Acceptor>(listenSock_, connections_, epoller_,receiver_,sender_,excepter_);
        //不小心把epoll的fd写入了内核，触发22错误
        AddConnection(listenSock_.Fd(), EPOLLIN | EPOLLET);

        eventLoop_ = std::make_shared<EventLoop>(epoller_,connections_, revs_);
    }

    void startServer(int timeout)
    {
        eventLoop_->loop(timeout);
    }

    void addRoute(const std::string &pattern, std::function<void(HttpRequest &, HttpResponse &)> handler){
        logger->info("注册路由: pattern %s", pattern.c_str());
        router_.addRoute(pattern, handler);
    }

protected:
    void AddConnection(int fd, uint32_t events, const std::string &ip = "127.0.0.1", const uint16_t &port = DEFAULT_PORT)
    {

        // 1 先判断是不是ET模式，如果是ET，则要设置为非阻塞
        if (events & EPOLLET)
        {
            FileUtils::setNonBlock(fd);
        }

        // 2 创建connection信息
        Connection *conn = new Connection(fd, ip, port);

        // 绑定监听方法
        conn->Register(acceptor_,nullptr,nullptr);
        // 记录连接关注的事件
        conn->events_ = events;

        connections_.insert({fd, conn});

        // 3 写入内核
        epoller_.AddModEvent(fd, conn->events_, EPOLL_CTL_ADD);
    }

private:
    uint16_t port_;
    std::unordered_map<int, Connection *> connections_;
    Sock listenSock_;
    Epoller epoller_;
    Router router_;
    epoll_event revs_[GNUM];

    std::shared_ptr<Acceptor> acceptor_;
    std::shared_ptr<Receiver> receiver_;
    std::shared_ptr<Sender> sender_;
    std::shared_ptr<Excepter> excepter_;

    std::shared_ptr<EventLoop> eventLoop_;

private:
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};