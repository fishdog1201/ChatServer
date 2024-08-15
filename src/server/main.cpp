#include <iostream>
#include <signal.h>
#include "chat_server.hpp"
#include "chat_service.hpp"

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main()
{
    signal(SIGINT, resetHandler);
    
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1", 50005);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}