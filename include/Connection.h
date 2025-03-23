#pragma once
#include <string>
#include <iostream>
#include <ctime>
#include <mysql/mysql.h>

using namespace std;

class Connection
{
public:
    //构造函数，初始化数据库连接
    Connection();
    //析构函数，释放数据库连接资源
    ~Connection();
    
    //连接数据库 
    bool connect(string ip,unsigned short port,string user,string password,string dbname);

    //更新操作 update insert delete
    bool update(string sql);

    //查询操作 select
    MYSQL_RES *query(string sql);
private:
    MYSQL *_conn;   //表示和mysql的多条连接
	clock_t _alivetime; 
};