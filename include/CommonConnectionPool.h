#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>
#include <functional>
#include "Connection.h"

using namespace std;

class ConnectionPool
{
public:
    //获取连接池对象实例
    static ConnectionPool* getConnectionPool();

    //给外部提供接口，从连接池中获取一个可用的空闲连接
    shared_ptr<Connection> getConnection();
    


private:
    ConnectionPool();   //单例：构造函数私有化
    bool loadConfigFile();   // 读取配置文件

    void produceConnectionTask();   //运行在独立的线程中，专门负责生产新连接

    void scannerConnectionTask();   // 运行在独立的线程中，专门负责扫描超过最大空闲时间的空闲连接，进行连接回收
   
    string ip_; // 数据库地址
    unsigned short port_;   // 数据库端口
    string userName_;  // 数据库用户名
    string passWd_;     // 数据库密码
    string dbName_;        // 数据库名
    int initSize_;      // 连接池初始化时，创建的连接个数
    int maxSize_;       // 连接池最大连接数
    int maxIdleTime_;       // 连接池最大空闲时间
    int connectTimeout_;        // 连接池获取连接的超时时间

    queue<Connection*> connectionQue_;      // 存储数据库连接队列
    mutex queMutex_;    //维护连接队列的互斥锁，保证线程安全
    atomic_int connectionCnt_;  //记录连接所创建Connection对象总数量
    condition_variable cv_; //设置条件变量，用于连接生产线程和连接消费线程的通信

};