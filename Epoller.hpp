#pragma once
#include <iostream>
#include <sys/epoll.h>
#include <cstring>
#include <cerrno>
#include <memory>

#include "Log/nhylog.hpp"

class Epoller
{
public:
    static const int default_epfd = -1;
    static const int default_gsize = 128;

    Epoller() : epfd_(default_epfd)
    {
        logger = nhylog::getRootLogger();
        if (!logger) {
            throw std::runtime_error("Logger initialization failed");
        }
    }

    ~Epoller()
    {
        Close();
    }

    void Create()
    {
        epfd_ = epoll_create(default_gsize);
        if (epfd_ < 0)
        {
            logger->fatal("Failed to create Epoller. Error: %d - %s", errno, strerror(errno));
            throw std::runtime_error("Failed to create Epoller");
        }
        logger->info("Epoller created successfully with epfd %d", epfd_);
    }

    bool AddModEvent(int fd, uint32_t events, int op)
    {
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = events;

        int n = epoll_ctl(epfd_, op, fd, &ev);
        if (n < 0)
        {

            logger->fatal("epoll_ctl error: %d - %s", errno, strerror(errno));
            return false;
        }
        return true;
    }

    bool DelEvent(int fd)
    {
        if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) != 0)
        {
            logger->fatal("Failed to delete event for fd %d: %d - %s", fd, errno, strerror(errno));
            return false;
        }

        logger->info("Event on fd %d was successfully deleted.", fd);
        return true;
    }

    int Wait(struct epoll_event *revs, int max_num, int timeout)
    {
        int nfds = epoll_wait(epfd_, revs, max_num, timeout);
        if (nfds < 0)
        {
            logger->fatal("epoll_wait error: %d - %s", errno, strerror(errno));
        }

        return nfds;
    }

    void Close()
    {
        if (epfd_ != default_epfd)
        {
            if (close(epfd_) == 0) {
                logger->info("Epoller with epfd %d was closed.", epfd_);
            } else {
                logger->fatal("Failed to close epfd %d: %d - %s", epfd_, errno, strerror(errno));
            }
            epfd_ = default_epfd;
        }
    }

    int Fd() const
    {
        return epfd_;
    }

private:
    int epfd_;
    std::shared_ptr<nhylog::Logger> logger;
};
