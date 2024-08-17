#pragma once

#include <string>
#include <vector>
#include "user.hpp"

class GroupUser
{
public:
    void setId(int id) 
    {
        this_user.setId(id);
    }
    void setName(std::string name) 
    {
        this_user.setName(name);
    }
    void setState(std::string state)
    {
        this_user.setState(state);
    }
    void setRole(std::string role) 
    {
        this->role = role;
    }
    std::string getRole()
    {
        return this->role;
    }
    int getId()
    {
        return this_user.getId();
    }
    std::string getName()
    {
        return this_user.getName();
    }
    std::string getState()
    {
        return this_user.getState();
    }
private:
    User this_user;
    std::string role;
};