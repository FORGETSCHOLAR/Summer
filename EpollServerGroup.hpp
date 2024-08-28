#pragma once
// 封装整个EpollServer，区分为EpollServerBoss 和 EpollServerWorker

#include "EpollServer.hpp"
#include <vector>
#include "EpollServerWorker.hpp"
#include "EpollServerBoss.hpp"

class EpollServerGroup
{
public:
private:
};

class EpollServerBossGroup : public EpollServerGroup
{
public:
private:
    std::vector<EpollServerBoss> boss_;
};

class EpollServerWorkerGroup : public EpollServerGroup
{
public:
private:
    std::vector<EpollServerWorker> workers_;
};