#pragma once

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include "json.hpp"

using json = nlohmann::json;
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr&, json&, muduo::Timestamp)>;

// service class of chat server
class ChatService {
private:
    ChatService();
public:
    static ChatService* instance();

    void login(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void reg(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    MsgHandler getHandler(int msgID);
private:
    std::unordered_map<int, MsgHandler> _msgHandlerMap;
};