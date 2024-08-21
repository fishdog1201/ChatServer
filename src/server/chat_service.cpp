#include "chat_service.hpp"
#include "friend_model.hpp"
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
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, 
                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, 
                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, 
                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, 
                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    
    if (_redis.connect()) {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, std::placeholders::_1,
                                   std::placeholders::_2));
    }
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
        reponse["errMsg"] = "Incorrect user id or password.";
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
        reponse["errMsg"] = "The User has already online.";
        conn->send(reponse.dump());
        return;
    }

    {
        std::lock_guard<std::mutex> lck(_connMutex);
        _userConnMap.insert({id, conn});
    }

    // After login success
    _redis.subscribe(id);

    user.setState("online");
    _userModel.updateState(user);
    reponse["msgid"] = LOGIN_MSG_ACK;
    reponse["errno"] = 0;
    reponse["id"] = user.getId();
    reponse["name"] = user.getName();
    offlineMsg = _offlineMsgModel.query(user.getId());
    if (!offlineMsg.empty()) {
        reponse["offlineMsg"] = offlineMsg;
        _offlineMsgModel.remove(user.getId());
    }
    std::vector<User> friends = _friendModel.query(user.getId());
    if (!friends.empty()) {
        std::vector<std::string> friends_vec;
        for (User& user: friends) {
            json js;
            js["id"] = user.getId();
            js["name"] = user.getName();
            js["state"] = user.getState();
            friends_vec.emplace_back(js.dump());
        }
        reponse["friends"] = friends_vec;
    }
    std::vector<Group> groups = _groupModel.queryGroups(user.getId());
    if (!groups.empty()) {
        std::vector<std::string> groups_vec;
        for (Group& group: groups) {
            json js;
            js["id"] = group.getId();
            js["groupName"] = group.getName();
            js["groupDesc"] = group.getDesc();

            std::vector<std::string> user_vec;
            for (GroupUser& users: group.getUsers()) {
                json js;
                js["id"] = users.getId();
                js["name"] = users.getName();
                js["state"] = users.getState();
                js["role"] = users.getRole();
                user_vec.emplace_back(js.dump());
            }

            groups_vec.emplace_back(js.dump());
        }
        reponse["groups"] = groups_vec;
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

    _redis.unsubscribe(user.getId());

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

    User user = _userModel.query(peerId);
    if (user.getState() == "online") {
        _redis.publish(user.getId(), js.dump());
        return;
    }

    //storage offline message
    _offlineMsgModel.insert(peerId, js.dump());

    return;
}

void ChatService::reset()
{
    _userModel.resetState();
}

void ChatService::addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time)
{
    int userId = js["id"].get<int>();
    int friendId = js["friendId"].get<int>();

    _friendModel.insert(userId, friendId);
    return;
}

void ChatService::createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time)
{
    int userId = js["id"].get<int>();
    std::string name = js["name"];
    std::string desc = js["desc"];

    Group group(-1, name, desc);
    if (_groupModel.createGroup(group) == false) {
        LOG_ERROR << "Create group failed!";
        return;
    }
    _groupModel.addGroup(userId, group.getId(), "creator");
    return;
}

void ChatService::addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time)
{
    int userId = js["id"].get<int>();
    int groupId = js["groupId"].get<int>();
    _groupModel.addGroup(userId, groupId, "normal");
    return;
}

void ChatService::groupChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time)
{
    int userId = js["id"].get<int>();
    int groupId = js["groupId"].get<int>();
    std::vector<int> groupUsers = _groupModel.queryGroupUsers(userId, groupId);
    std::lock_guard<std::mutex> lock(_connMutex);
    for (int id: groupUsers) {
        auto it = _userConnMap.find(id);
        if (it == _userConnMap.end()) {
            User user = _userModel.query(id);
            if (user.getState() == "online") {
                _redis.publish(id, js.dump());
            } else {
                _offlineMsgModel.insert(id, js.dump());
            }
        } else {
            it->second->send(js.dump());
        }
    }
    return;
}

void ChatService::handleRedisSubscribeMessage(int userId, std::string message)
{
    json js = json::parse(message.c_str());

    std::lock_guard<std::mutex> lck(_connMutex);
    auto it = _userConnMap.find(userId); 
    if (it != _userConnMap.end()) {
        it->second->send(js.dump());
        return;
    }

    _offlineMsgModel.insert(userId, message);
    return;
}