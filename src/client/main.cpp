#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>

#include "json.hpp"
#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

using json = nlohmann::json;

User g_currentUser;
std::vector<User> g_currentUserFriends;
std::vector<Group> g_currentUserGroups;

void showCurUserInfo();
void readTaskHandler(int clientFd);
std::string getCurrentTime();
void mainMenu();

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Invalid command. Example: ./chatClient 127.0.0.1 50005\n";
        exit(-1);
    }

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1) {
        std::cerr << "socket create failed!\n";
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(clientFd, (sockaddr*)&server, sizeof(sockaddr_in)) == -1) {
        std::cerr << "Connect to  server failed!\n";
        close(clientFd);
        exit(-1);
    }

    for(;;) {
        std::cout << "===============\n";
        std::cout << "1. login\n";
        std::cout << "2. register\n";
        std::cout << "3. quit\n";
        std::cout << "===============\n";
        std::cout << "choice: ";
        int choice = 0;
        std::cin >> choice;
        std::cin.get();

        switch (choice) {
            case 1:
            {
                int id = 0;
                char pwd[50] = {0};
                std::cout << "userId: ";
                std::cin >> id;
                std::cin.get();
                std::cout << "Password: ";
                std::cin.getline(pwd, 50);

                json js;
                js["msgId"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                std::string request = js.dump();

                int len = send(clientFd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1) {
                    std::cerr << "Send login message failed!\n";
                } else {
                    char buffer[1024] = {0};
                    len = recv(clientFd, buffer, 1024, 0);
                    if (len == -1) {
                        std::cerr << "receive login message reponse failed!\n";
                    } else {
                        json reponse = json::parse(buffer);
                        if (reponse["errno"].get<int>() != 0) {
                            std::cerr << reponse["errMsg"] << std::endl;
                        } else {
                            g_currentUser.setId(reponse["id"].get<int>());
                            g_currentUser.setName(reponse["name"]);
                            if (reponse.contains("friends")) {
                                std::vector<std::string> vec = reponse["friends"];
                                for (auto& str: vec) {
                                    json js = json::parse(str);
                                    User user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    g_currentUserFriends.emplace_back(user);
                                }
                            }
                            if (reponse.contains("groups")) {
                                std::vector<std::string> vec = reponse["groups"];
                                for (auto& str: vec) {
                                    json js = json::parse(str);
                                    Group group;
                                    group.setId(js["id"].get<int>());
                                    group.setName(js["groupName"]);
                                    group.setDesc(js["groupDesc"]);

                                    std::vector<std::string> groupUsers = js["users"];
                                    for (std::string& userStr: groupUsers) {
                                        GroupUser user;
                                        json js = json::parse(userStr);
                                        user.setRole(js["role"]);
                                        user.setId(js["id"].get<int>());
                                        user.setName(js["name"]);
                                        user.setState(js["state"]);
                                        group.getUsers().push_back(user);
                                    }
 
                                    g_currentUserGroups.emplace_back(group);
                                }
                            }

                            showCurUserInfo();

                            if (reponse.contains("offlineMsg")) {
                                std::vector<std::string> offlineMsgs = reponse["offlineMsg"];
                                for (std::string& str: offlineMsgs) {
                                    json js = json::parse(str);
                                    std::cout << js["time"] << " [" << js["id"] << "]" << js["name"] 
                                              << " said: " << js["message"] << std::endl;
                                }
                            }

                            std::thread readTask(readTaskHandler, clientFd);
                            readTask.detach();

                            mainMenu(clientFd);
                        }
                    }
                }

                break;
            }
            case 2:
            {
                char name[50] = {0};
                char pwd[50] = {0};
                std::cout << "UserName: ";
                std::cin.getline(name, 50);
                std::cout << "Password: ";
                std::cin.getline(pwd, 50);

                json js;
                js["msgId"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                std::string request = js.dump();

                int len = send(clientFd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1) {
                    std::cerr << "Send register message failed!\n";
                } else {
                    char buffer[1024] = {0};
                    len = recv(clientFd, buffer, 1024, 0);
                    if (len == -1) {
                        std::cerr << "receive register message reponse failed!\n";
                    } else {
                        json reponse = json::parse(buffer);
                        if (reponse["errno"].get<int>() != 0) {
                            std::cerr << name << " already exists, please change it.\n";
                        } else {
                            std::cout << "Register success. User id is " << reponse["id"] << ", don't forget it!\n";
                        }
                    }
                }
                break;
            }
            case 3:
            {
                close(clientFd);
                exit(0);
            }
            default:
            {
                std::cerr << "Invalid choice!\n";
                break;
            }
        }
    }
}

void showCurUserInfo()
{
    std::cout << "===================Login User===================\n";
    std::cout << "Current user's id = " << g_currentUser.getId() <<
                 ", name = " << g_currentUser.getName() << std::endl;
    std::cout << "===================Friends===================\n";
    if (!g_currentUserFriends.empty()) {
        for (User& user: g_currentUserFriends) {
            std::cout << user.getId() << " :: " << user.getName() << " :: " << user.getState() << std::endl; 
        }
    }
    std::cout << "===================Groups====================\n";
    if (!g_currentUserGroups.empty()) {
        for (Group& group: g_currentUserGroups) {
            std::cout << group.getId() << " :: " << group.getName() << std::endl;
            for (GroupUser& groupUsers: group.getUsers()) {
                std::cout << "    ";
                std::cout << groupUsers.getId() << " :: " << groupUsers.getName() << " :: " 
                          << groupUsers.getState() << " :: " << groupUsers.getRole() << std::endl;
            }
        }
    }
}

void readTaskHandler(int clientFd)
{
    for (;;) {
        char buffer[1024] = {0};
        int len = recv(clientFd, buffer, 1024, 0);
        if (len == -1 || len == 0) {
            close(clientFd);
            exit(-1);
        }

        json js = json::parse(buffer);
        if (js["msgId"].get<int>() == ONE_CHAT_MSG) {
            std::cout << js["time"] << " [" << js["id"] << "]" << js["name"] 
                      << " said: " << js["message"] << std::endl;
            continue;
        }
    }
}

std::string getCurrentTime()
{
    return "";
}

void help(int, std::string);
void chat(int, std::string);
void addfriend(int, std::string);
void creategroup(int, std::string);
void joingroup(int, std::string);
void groupchat(int, std::string);
void logout(int, std::string);

std::unordered_map<std::string, std::string> commandMap = {
    {"help", "Show all the commands, format: help"},
    {"chat", "peer to peer chat, format: chat:friendId:message"},
    {"addfriend", "Add friend, format: addfriend:friendId"},
    {"creategroup", "create group, format: creategroup:groupName:groupDesc"},
    {"joingroup", "Join group, format: joingroup:groupId"},
    {"groupchat", "Group chat, format: groupchat:groupId:message"},
    {"logout", "Logout, format: logout"}
};

std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"joingroup", joingroup},
    {"groupchat", groupchat},
    {"logout", logout}
};

void mainMenu(int clientFd)
{
    help(clientFd, 0);

    char buffer[1024] = {0};
    for (;;) {
        std::cin.getline(buffer, 1024);
        std::string commandBuffer(buffer);
        std::string command;
        int idx = commandBuffer.find(":");
        if (idx == -1) {
            command = commandBuffer;
        } else {
            command = commandBuffer.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end()) {
            std::cerr << "Invalid command!\n";
            continue;
        }

        it->second(clientFd, commandBuffer.substr(idx + 1, commandBuffer.size() - idx));
    }
    return;
}

void help(int clientFd, std::string str)
{
    std::cout << "Show command list >>> " << std::endl;
    for (auto& p: commandMap) {
        std::cout << p.first << " :: " << p.second << std::endl;
    }
    std::cout << std::endl;
}

void addfriend(int clientFd, std::string str)
{
    int friendId = atoi(str.c_str());
    json js;
    js["msgId"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendId"] = friendId;
    std::string buffer = js.dump();

    int len = send(clientFd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "Send add friend msg error -> " << buffer << std::endl;
    }
}

// Other function of client omitted