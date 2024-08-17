#include "group_model.hpp"
#include "db.h"
#include <muduo/base/Logging.h>

bool GroupModel::createGroup(Group& group)
{
    char sql[1024];
    sprintf(sql, "insert into allgroup(groupName, groupDesc) values('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());
    
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql) == false) {
            LOG_ERROR << "Insert group to db failed.";
            return false;
        }
    }

    group.setId(mysql_insert_id(mysql.getConnection()));
    return true;
}

void GroupModel::addGroup(int userId, int groupId, std::string role)
{
    char sql[1024];
    sprintf(sql, "insert into groupuser values(%d, %d, '%s')",
            groupId, userId, role.c_str());
    
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql) == false) {
            LOG_ERROR << "Insert group user to db failed.";
        }
    }
    return;
}

std::vector<Group> GroupModel::queryGroups(int userId)
{
    char sql[1024] = {0};
    std::vector<Group> groups;
    MySQL mysql;

    sprintf(sql, "select a.id,a.groupName,a.groupDesc from allgroup a inner join \
                  groupuser b on b.groupId = a.id where b.userId = %d", userId);

    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res == nullptr) {
            LOG_ERROR << "query groups by id failed.";
            return groups;
        }
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr) {
            Group my_groups;
            my_groups.setId(atoi(row[0]));
            my_groups.setName(row[1]);
            my_groups.setDesc(row[2]);
            groups.emplace_back(my_groups);
        }
        mysql_free_result(res);
    }

    return groups;
}

std::vector<int> GroupModel::queryGroupUsers(int userId, int groupId)
{
    char sql[1024] = {0};
    std::vector<int> ids;
    MySQL mysql;

    sprintf(sql, "select userId from groupuser where groupId = %d and userId = %d", groupId, userId);

    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res == nullptr) {
            LOG_ERROR << "query group users by id failed.";
            return ids;
        }
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr) {
            ids.emplace_back(row[0]);
        }
        mysql_free_result(res);
    }

    return ids;
}