#include "db.h"
#include <muduo/base/Logging.h>

MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if (_conn != nullptr) {
        mysql_close(_conn);
    }
}

bool MySQL::connect()
{
    MYSQL* p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                      password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p == nullptr) {
        LOG_INFO << "connect mysql falied.";
        fprintf(stderr, "Failed to connect to database: Error: %s\n",
          mysql_error(_conn));
        return p;
    }
    mysql_query(_conn, "set names gbk");
    LOG_INFO << "connect mysql successfully.";
    return p;
}

bool MySQL::update(std::string sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ": " << __LINE__ << ": " << sql << " UPDATE FAILED!";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(std::string sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ": " << __LINE__ << ": " << sql << " QUERY FAILED!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL* MySQL::getConnection()
{
    return _conn;
}
