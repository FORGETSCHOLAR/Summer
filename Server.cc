#include <iostream>


#include "EpollServer.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

void HelloController(HttpRequest& request, HttpResponse& response){
    response.setStatus(200,"OK");
    response.addHeader("Content-Type", "text/plain; charset=utf-8");
    response.setBody("有这时间多刷两道题不行？");
}

int main(){
    EpollServer server{};
    server.initServer();
    server.addRoute("/hello",HelloController);
    server.startServer(2000);

    return 0;
}