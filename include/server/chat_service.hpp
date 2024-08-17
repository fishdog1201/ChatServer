#pragma once

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "json.hpp"
#include "user_model.hpp"
#include "offline_msg_model.hpp"
#include "friend_model.hpp"
#include "group_model.hpp"

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

    void oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void groupChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);

    void clientCloseException(const muduo::net::TcpConnectionPtr& conn);

    void reset();

    MsgHandler getHandler(int msgID);
private:
    std::unordered_map<int, MsgHandler> _msgHandlerMap;
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnMap;
    std::mutex _connMutex;
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
};