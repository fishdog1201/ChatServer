#pragma once

#include <vector>
#include "user.hpp"

class FriendModel {
public:
    void insert(int userId, int friendId);

    std::vector<User> query(int userId);
};