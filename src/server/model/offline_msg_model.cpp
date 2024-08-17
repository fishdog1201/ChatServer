#include "offline_msg_model.hpp"
#include "db.h"
#include <muduo/base/Logging.h>

void OfflineMsgModel::insert(int userId, std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s')", userId, msg.c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql) == false) {
            LOG_ERROR << "Insert offline message to db failed.";
            return;
        }
    }
    return;
}
void OfflineMsgModel::remove(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userId=%d", userId);
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql) == false) {
            LOG_ERROR << "delete offline message by user id failed.";
            return;
        }
    }
    return;
}
std::vector<std::string> OfflineMsgModel::query(int userId)
{
    char sql[1024] = {0};
    std::vector<std::string> messages;
    MySQL mysql;

    sprintf(sql, "select message from offlinemessage where userId = %d", userId);

    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res == nullptr) {
            LOG_ERROR << "query user by id failed.";
            return messages;
        }
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr) {
            messages.emplace_back(row[0]);
        }
        mysql_free_result(res);
    }

    return messages;
}