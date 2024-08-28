#pragma once

#include "IHandler.hpp"
#include "../Log/nhylog.hpp"
#include "../HttpRequest.hpp"
#include "../HttpResponse.hpp"
#include "../Router.hpp"

class Receiver : public IEventHandler
{
public:
    Receiver(Router &router):router_(router)
    {
    }

    void handle(Connection *conn) override
    {

        if (ReceiverHelper(conn) == false)
        {
            return;
        }

        // 可以在这里处理接收到的数据
        // 1 有一个组件能够从inbuffer里读取一个完整的http报文,提取一个完整的，就把这部分删掉，不完整就先不处理等完整在处理
        std::string parsedHttpRequest;
        if (!HttpRequest::extractCompleteRequest(conn->inbuffer_, parsedHttpRequest))
        {
            logger->info("Receiver::未能读取到一个完整报文");
            return;
        }

        // 2 然后将request报文给另一个组件，比如是专门处理请求的线程池,这里暂时就在这里
        HttpRequest httpRequest = HttpRequest{};
        httpRequest.parse(parsedHttpRequest);
        logger->info("解析请求成功: %s", httpRequest.getUri().c_str());

        HttpResponse httpResponse = HttpResponse{};
        router_.route(httpRequest, httpResponse);

        // 3 序列化响应
        std::string parsedHttpResponse;
        parsedHttpResponse = httpResponse.toString();
        logger->info("序列化响应完成");
        

        // 4 先尝试写回
        if (!parsedHttpResponse.empty())
        {
            conn->outbuffer_ += parsedHttpResponse;
            conn->handleWrite();      
        }
    }

private:
    bool ReceiverHelper(Connection *conn)
    {
        do
        {
            char buffer[DEFAULT_BUFFER_SIZE];
            int size = recv(conn->fd_, buffer, sizeof(buffer) - 1, 0); // 0表示默认行为，即fd若为阻塞则阻塞等待有内容，若不为阻塞则直接返回-1，并将errno设置为EAGAIN
            if (size > 0)
            {
                buffer[size] = 0;
                conn->inbuffer_ += buffer;
            }
            else if (size == 0)
            {
                logger->info("receiveHelper::对端关闭了连接");
                conn->handleException();
                return false;
            }
            else
            {

                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 没有数据可读
                    break;
                }
                else if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    logger->error("receverHelper发生错误 %d %s", errno, strerror(errno));
                    conn->handleException();
                    return false;
                }
            }
        } while (conn->events_ & EPOLLET);

        return true;
    }

private:
    Router &router_;
    nhylog::Logger::ptr logger = nhylog::getRootLogger();
};
