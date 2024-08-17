#pragma once

#include <string>
#include <vector>

class GroupUser
{
public:
    void setRole(std::string role) 
    {
        this->role = role;
    }
    std::string getRole()
    {
        return this->role;
    }
private:
    std::string role;
};