#include <iostream>
#include "Connection.h"
#include "CommonConnectionPool.h"
int main()
{
    string sql = "insert into user(name,age,sex) values('123456','123','male');";
    #if 0
    //未使用连接池，单线程测试
    clock_t start = clock();
    
    for(int i = 0;i < 10000;i++)
    {
        Connection conn;
        conn.connect("127.0.0.1",3306,"shaohua","010407","chat");
        if(!conn.update(sql))
        {
            std::cout << "insert error" << std::endl;
        }
    }
    clock_t end = clock();

    cout<<"use time:"<<(end - start)<<"ms"<<endl;
    #endif
    
    #if 0
    clock_t start = clock();
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    for(int i = 0;i < 10000;i++)
    {
        shared_ptr<Connection> conn = pool->getConnection();
        conn->update(sql);
    }
     clock_t end = clock();

    cout<<"use time:"<<(end - start)<<"ms"<<endl;
    #endif

    #if 1
    clock_t start = clock();
    thread t1([&](){
        ConnectionPool* pool = ConnectionPool::getConnectionPool();
        for(int i = 0;i < 2500;i++)
        {
            shared_ptr<Connection> conn = pool->getConnection();
            conn->update(sql);
        }

    });
    thread t2([&](){
        ConnectionPool* pool = ConnectionPool::getConnectionPool();
        for(int i = 0;i < 2500;i++)
        {
            shared_ptr<Connection> conn = pool->getConnection();
            conn->update(sql);
        }

    });
    thread t3([&](){
        ConnectionPool* pool = ConnectionPool::getConnectionPool();
        for(int i = 0;i < 2500;i++)
        {
            shared_ptr<Connection> conn = pool->getConnection();
            conn->update(sql);
        }
    });
    thread t4([&](){
        ConnectionPool* pool = ConnectionPool::getConnectionPool();
        for(int i = 0;i < 2500;i++)
        {
            shared_ptr<Connection> conn = pool->getConnection();
            conn->update(sql);
        }
    });
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    clock_t end = clock();

    cout<<"use time:"<<(end - start)<<"ms"<<endl;

    #endif


    return 0;
}