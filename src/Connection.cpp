#include "public.h"
#include "Connection.h"

//构造函数，初始化数据库连接
Connection::Connection()
{
    //初始化数据库连接
    conn_ = mysql_init(nullptr);
}
//析构函数，释放数据库连接资源
Connection::~Connection()
{
    if(conn_ != nullptr)
        mysql_close(conn_);
}

//连接数据库 
bool Connection::connect(string ip,unsigned short port,string user,string password,string dbname)
{
    MYSQL *p = mysql_real_connect(conn_,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
    return p != nullptr;
}

//更新操作 update insert delete
bool Connection::update(string sql)
{
    if(mysql_query(conn_,sql.c_str()))
    {
        LOG("Query error:" + sql);
        return false;
    }
    return true;
}

//查询操作 select
MYSQL_RES * Connection::query(string sql)
{
    if(mysql_query(conn_,sql.c_str()))
    {
        LOG("Query error:" + sql);
        return nullptr;
    }
    return mysql_use_result(conn_);
}