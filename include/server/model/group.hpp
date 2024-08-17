#pragma once

#include "group_user.hpp"

class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
        :id(id), name(name), desc(desc) {}
    
    void setId(int id) 
    {
        this->id = id;
    }

    void setName(std::string name) 
    {
        this->name = name;
    }

    void setDesc(std::string desc)
    {
        this->desc = desc;
    }

    int getId()
    {
        return this->id;
    }

    std::string getName()
    {
        return this->name;
    }

    std::string getDesc()
    {
        return this->desc;
    }

    std::vector<GroupUser>& getUsers()
    {
        return this->_groupUsers;
    }
    
private:
    int id;
    std::string name;
    std::string desc;
    std::vector<GroupUser> _groupUsers;
};