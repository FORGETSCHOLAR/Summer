#include <iostream>

#include "Connection.hpp"
#include "EpollServer.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

//测试基础功能
void HelloController(HttpRequest& request, HttpResponse& response){
    response.setStatus(200,"OK");
    response.addHeader("Content-Type", "text/plain; charset=utf-8");
    response.setBody("有这时间多刷两道题不行？");
}

//测试postParam功能
void HiController(HttpRequest &request, HttpResponse &response)
{
    auto name =  request.getPostParametersMultiple("name");
    auto pass = request.getPostParametersMultiple("pass");
    response.setStatus(200, "OK");
    response.addHeader("Content-Type", "text/plain; charset=utf-8");
    response.setBody(name[0] + " " +pass[0]);
}

void UserController(HttpRequest &request, HttpResponse &response)
{
    auto id = request.getQueryParametersMultiple("id");

    if(id.size() == 0){
        response.setBody("没有找到参数id");
    }else{
        response.setBody("获得用户信息 "+id[0]);
    }

    response.setStatus(200, "OK");
    response.addHeader("Content-Type", "text/plain; charset=utf-8");
}

//测试文件功能
void FileController(){

}

int main(){
    EpollServer server{};
    server.initServer();
    server.addRoute("/test/hello", HelloController);
    server.addRoute("/test/hi", HiController);
    server.addRoute("/user/{id}", UserController);
    server.startServer(1000);

    return 0;
}