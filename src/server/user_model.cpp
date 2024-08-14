#include "user_model.hpp"
#include "db.h"
#include <iostream>
#include <muduo/base/Logging.h>

bool UserModel::insert(User& user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql) == false) {
            LOG_ERROR << "Insert user to db failed.";
            return false;
        }
    }

    user.setId(mysql_insert_id(mysql.getConnection()));
    return true;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    MySQL mysql;
    User user;

    sprintf(sql, "select * from user where id = %d", id);

    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res == nullptr) {
            LOG_ERROR << "query user by id failed.";
            return user;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == nullptr) {
            mysql_free_result(res);
            LOG_ERROR << "fetch row by id failed.";
            return user;
        }

        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setPwd(row[2]);
        user.setState(row[3]);

        mysql_free_result(res);
    }

    return user;
}

bool UserModel::updateState(User& user)
{
    char sql[1024] = {0};
    MySQL mysql;

    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    if (mysql.connect()) {
        if (mysql.update(sql) == false) {
            LOG_ERROR << "Set user state failed.";
            return false;
        }
    }

    return true;
}