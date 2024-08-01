#include "chat_server.hpp"
#include <functional>


ChatServer::ChatServer(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& listenAddr,
            const std::string& nameArg)
            : _server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, 
                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _server.setThreadNum(8);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& )
{}

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr&,
                           muduo::net::Buffer*,
                           muduo::Timestamp)
{}