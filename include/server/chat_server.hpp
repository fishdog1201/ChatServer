#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

class ChatServer
{
public:
    // Init chat server instance
    ChatServer(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& listenAddr,
            const std::string& nameArg);
    
    // Start service
    void start();
private:
    // Callback function related to connection
    void onConnection(const muduo::net::TcpConnectionPtr&);
    // Callback function related to message
    void onMessage(const muduo::net::TcpConnectionPtr&,
                            muduo::net::Buffer*,
                            muduo::Timestamp);

    muduo::net::TcpServer _server; // muduo lib, server function
    muduo::net::EventLoop* _loop;  // muduo lib, point to event loop
};