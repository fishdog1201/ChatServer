#pragma once

#include "user.hpp"

class UserModel {
public:
    bool insert(User& user); // Add record to db

    User query(int id);

    bool updateState(User& user);

    void resetState();
};