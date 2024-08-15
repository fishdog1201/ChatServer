#include "chat_service.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include <mutex>

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
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, 
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
    int id = js["id"];
    std::string pwd = js["password"];
    json reponse;
    std::vector<std::string> offlineMsg;

    User user = _userModel.query(id);
    if (user.getId() == -1 || user.getPwd() != pwd) {
        reponse["msgid"] = LOGIN_MSG_ACK;
        reponse["errno"] = 1;
        reponse["msg"] = "Incorrect user id or password.";
        conn->send(reponse.dump());
        return;
    }
    if (user.getState() == "online") {
        {
            std::lock_guard<std::mutex> lck(_connMutex);
            _userConnMap.insert({id, conn});
        }
        reponse["msgid"] = REG_MSG_ACK;
        reponse["errno"] = 2;
        reponse["msg"] = "The User has already online.";
        conn->send(reponse.dump());
        return;
    }

    {
        std::lock_guard<std::mutex> lck(_connMutex);
        _userConnMap.insert({id, conn});
    }
    user.setState("online");
    _userModel.updateState(user);
    reponse["msgid"] = LOGIN_MSG_ACK;
    reponse["errno"] = 0;
    reponse["id"] = user.getId();
    reponse["name"] = user.getName();
    offlineMsg = _offlineMsgModel.query(user.getId());
    if (!offlineMsg.empty()) {
        reponse["offlinemsg"] = offlineMsg;
        _offlineMsgModel.remove(user.getId());
    }
    conn->send(reponse.dump());
    return;
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
    return;
}

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& conn)
{
    User user;

    std::lock_guard<std::mutex> lck(_connMutex);
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
        if (it->second == conn) {
            user.setId(it->first);
            _userConnMap.erase(it->first);
            break;
        }
    }

    user.setState("offline");
    _userModel.updateState(user);
}

void ChatService::oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time)
{
    int peerId = js["to"].get<int>();
    {
        std::lock_guard<std::mutex> lck(_connMutex);    
        auto it = _userConnMap.find(peerId);
        if (it != _userConnMap.end()) {
            it->second->send(js.dump());
            return;
        }
    }

    // TODO: storage offline message
    _offlineMsgModel.insert(peerId, js.dump());

    return;
}

void ChatService::reset()
{
    _userModel.resetState();
}