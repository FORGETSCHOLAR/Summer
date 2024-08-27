#pragma once

#include <iostream>
#include <memory>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Log/nhylog.hpp"
#include "Exception/Exception.hpp"

class Sock
{
public:
    static const int defaultfd = -1;
    static const int gbacklog = 32;

    Sock(int fd = defaultfd) : _sock(fd)
    {
        
    }

    ~Sock()
    {
        Close();
    }

    void Socket()
    {
        _sock = socket(AF_INET, SOCK_STREAM, 0);
        if (_sock < 0)
        {
            logger->fatal("Create socket error: %d - %s", errno, strerror(errno));
            throw std::runtime_error("Failed to create socket");
        }

        int optval = 1;
        if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0)
        {
            logger->error("Setsockopt error: %d - %s", errno, strerror(errno));
            Close();
            throw std::runtime_error("Failed to set socket options");
        }

        logger->info("Socket created with fd %d", _sock);
    }

    void Bind(const uint16_t port)
    {
        sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = INADDR_ANY;

        if (bind(_sock, (sockaddr *)&local, sizeof(local)) < 0)
        {
            logger->error("Bind error: %d - %s", errno, strerror(errno));
            throw std::runtime_error("Bind failed");
        }

        logger->info("Bind success on port %d", port);
    }

    void Listen()
    {
        if (listen(_sock, gbacklog) < 0)
        {
            logger->error("Listen error: %d - %s", errno, strerror(errno));
            throw std::runtime_error("Listen failed");
        }

        logger->info("Listen success with backlog %d", gbacklog);
    }

    int Accept(std::string *clientip, uint16_t *clientport)
    {
        sockaddr_in temp;
        socklen_t len = sizeof(temp);
        int client_sock = accept(_sock, (sockaddr *)&temp, &len);
        if (client_sock < 0)
        {
            logger->error("Accept error: %d - %s", errno, strerror(errno));
            throw AcceptException(errno, strerror(errno)); 
            return -1;
        }

        *clientip = inet_ntoa(temp.sin_addr);
        *clientport = ntohs(temp.sin_port);
        
        logger->info("Accept success from %s:%d", clientip->c_str(), *clientport);
        logger->info("sock= %d",client_sock);


        return client_sock;
    }

    void Connect(const std::string &serverip, uint16_t serverport)
    {
        sockaddr_in server;
        memset(&server, 0, sizeof(server));

        server.sin_family = AF_INET;
        server.sin_port = htons(serverport);
        if (inet_pton(AF_INET, serverip.c_str(), &server.sin_addr) <= 0)
        {
            logger->error("Invalid address: %s - %s", serverip.c_str(), strerror(errno));
            throw std::invalid_argument("Invalid server IP address");
        }

        if (connect(_sock, (sockaddr *)&server, sizeof(server)) < 0)
        {
            logger->error("Connect error: %d - %s", errno, strerror(errno));
            throw std::runtime_error("Connection failed");
        }

        logger->info("Connected to server %s:%d", serverip.c_str(), serverport);
    }

    int Fd() const
    {
        return _sock;
    }

private:


    void Close()
    {
        if (_sock != defaultfd)
        {
            if (close(_sock) == 0)
            {
                logger->info("Socket with fd %d closed.", _sock);
            }
            else
            {
                logger->error("Failed to close socket with fd %d: %d - %s", _sock, errno, strerror(errno));
            }
            _sock = defaultfd;
        }
    }

    int _sock;
    std::shared_ptr<nhylog::Logger> logger = nhylog::getRootLogger();  // 使用日志实例
};
