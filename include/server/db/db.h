#pragma once

#include <mysql/mysql.h>
#include <string>

static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "123456";
static std::string dbname = "chat";

class MySQL
{
public:
    MySQL() ;
    ~MySQL();
    bool connect();
    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);
private:
    MYSQL* _conn;
};