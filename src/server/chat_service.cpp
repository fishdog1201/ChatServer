#include "chat_service.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, 
                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, 
                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
}

MsgHandler ChatService::getHandler(int msgID)
{
    auto it = _msgHandlerMap.find(msgID);
    if (it == _msgHandlerMap.end()) {
        return [=](const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time) {
            LOG_ERROR << "Message ID: " << msgID << " doesn't have a related handler!";
        };
    }
    return _msgHandlerMap[msgID];
}

void ChatService::login(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time)
{
    LOG_INFO << "Login service";
}

void ChatService::reg(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];
    json reponse;

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    
    if (state) {
        reponse["msgid"] = REG_MSG_ACK;
        reponse["errno"] = 0;
        reponse["id"] = user.getId();
        conn->send(reponse.dump());
    } else {
        reponse["msgid"] = REG_MSG_ACK;
        reponse["errno"] = 1;
        conn->send(reponse.dump());
    }
}