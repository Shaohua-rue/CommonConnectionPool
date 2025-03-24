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
    
    //刷新一下连接的起始的空闲事件点
    void refershAliveTime(){alivetime_=clock();};
    //返回存活的时间
    clock_t getAliveTime()const{return clock()-alivetime_;};
private:
    MYSQL *conn_;   //表示和mysql的一条连接
	clock_t alivetime_;     //记录连接进入空闲状态的存活时间
};  