#pragma once


class Connection;
class IEventHandler
{
public:
    virtual ~IEventHandler() = default;
    virtual void handle(Connection *conn) = 0;
};

