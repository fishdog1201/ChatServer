#pragma once

#include "group.hpp"

class GroupModel
{
public:
    bool createGroup(Group& group);

    void addGroup(int userId, int groupId, std::string role);

    std::vector<Group> queryGroups(int userId);

    std::vector<int> queryGroupUsers(int userId, int groupId);
};