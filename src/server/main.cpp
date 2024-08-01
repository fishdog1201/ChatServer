#include <iostream>
#include "chat_server.hpp"

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1", 50005);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}