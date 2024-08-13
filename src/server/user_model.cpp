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
        if (mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    LOG_INFO << "Insert user to db failed.";
    return false;
}