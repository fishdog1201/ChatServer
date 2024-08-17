#include <muduo/base/Logging.h>
#include "friend_model.hpp"
#include "db.h"

void FriendModel::insert(int userId, int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, '%d')", userId, friendId);
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql) == false) {
            LOG_ERROR << "Insert offline message to db failed.";
            return;
        }
    }
    return;
}

std::vector<User> FriendModel::query(int userId)
{
    char sql[1024] = {0};
    std::vector<User> friends;
    MySQL mysql;

    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendId = a.id where b.userId = %d", userId);

    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res == nullptr) {
            LOG_ERROR << "query user by id failed.";
            return friends;
        }
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr) {
            User my_friend;
            my_friend.setId(atoi(row[0]));
            my_friend.setName(row[1]);
            my_friend.setState(row[2]);
            friends.emplace_back(my_friend);
        }
        mysql_free_result(res);
    }

    return friends;
}