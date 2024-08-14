#include "chat_server.hpp"
#include "chat_service.hpp"
#include "json.hpp"
#include <functional>
#include <string>
#include <muduo/base/Logging.h>

using json = nlohmann::json;

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

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buffer,
                           muduo::Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    if (buf.size() == 0) {
        LOG_ERROR << "Empty json.";
        return;
    }
    json js = json::parse(buf);

    auto msgHandler = ChatService::instance()->getHandler(js["msgId"].get<int>());
    msgHandler(conn, js, time);
    return;
}