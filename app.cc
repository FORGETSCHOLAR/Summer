#include "EpollServerBoss.hpp"
#include "EpollServerWorker.hpp"
#include "Router.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <thread>

// 测试基础功能
void HelloController(HttpRequest &request, HttpResponse &response)
{
    response.setStatus(200, "OK");
    response.addHeader("Content-Type", "text/plain; charset=utf-8");
    response.setBody("有这时间多刷两道题不行？");
}

int main(){
    Router router;
    router.addRoute("/hello", HelloController);
    EpollServerWorker worker1(router);
    EpollServerWorker worker2(router);
    EpollServerWorker worker3(router);
    worker1.init();
    worker2.init();
    worker3.init();

    EpollServerBoss boss{};
    boss.init(8080);
    boss.interview(worker1);
    boss.interview(worker2);
    boss.interview(worker3);
    std::thread work1t(&EpollServerWorker::start, &worker1, 1000);
    std::thread work2t(&EpollServerWorker::start, &worker2,1000);
    std::thread work3t(&EpollServerWorker::start, &worker3, 1000);
    std::thread bosst(&EpollServerBoss::start, &boss,1000);

    work1t.join();
    work2t.join();
    work3t.join();
    bosst.join();

    return 0;
}
