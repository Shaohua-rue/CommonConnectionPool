#include "public.h"
#include "CommonConnectionPool.h"

//连接池的构造
ConnectionPool::ConnectionPool()
{
    //加载配置项
    if(!loadConfigFile())
    {
        return;
    }
    //创建初始数量的连接
    for(int i = 0; i < initSize_; i++)
    {
        Connection *p = new Connection();
        
        p->connect(ip_,port_,userName_,passWd_,dbName_);
        p->refershAliveTime();  //刷新开始空闲的起始时间
        connectionQue_.push(p);
        connectionCnt_++;
    }

    //启动一个新线程，作为连接的生产者线程
    thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();
    //启动一个新的定时线程，扫描超过maxIdleTime_时间的空闲连接，进行队列的连接回收
    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

//线程安全的懒汉单例模式接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::loadConfigFile()
{
    FILE *fp = fopen("../config/mysql.ini","r");
    if(fp == nullptr)
    {
        LOG("mysql.ini open error");
        return false;
    }

    while (!feof(fp))//当文件未到末尾
    {
        ///读取一行数据
        char line[1024] = {0};  
        fgets(line,1024,fp);
        string str = line;
        int index = str.find("=", 0);

        if(index == -1) //无效的配置项
        {
            continue;
        }

        int endIndex = str.find("\n", index);
        string key = str.substr(0, index);
        string value = str.substr(index + 1, endIndex - index - 1);
        
        if(key == "ip")
        {
            ip_ = value;
        }
        else if(key == "port")
        {
            port_ = atoi(value.c_str());
        }
        else if(key == "username")
        {
            userName_ = value;
        }
        else if(key == "password")
        {
            passWd_ = value;
        }
        else if(key == "dbname")
        {
            dbName_ = value;
        }
        else if(key == "initSize")
        {
            initSize_ = atoi(value.c_str());
            
        }
        else if(key == "maxSize")
        {
            maxSize_ = atoi(value.c_str());
        }
        else if(key == "maxIdleTime")
        {
            maxIdleTime_ = atoi(value.c_str());
        }
        else if(key == "connectionTimeout")
        {
            connectTimeout_ = atoi(value.c_str());
        }

    }
    return true;
}

//外部提供接口,从连接池中获取一个可用连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(queMutex_);
    // while(connectionQue_.empty())
    // {
    //     if(cv_status::timeout == cv_.wait_for(lock,chrono::milliseconds(connectTimeout_)))
    //     {
    //         if(connectionQue_.empty())
    //         {
    //             LOG("获取连接超时");
    //             return nullptr;
    //         }
    //     }
    // }

    if( !cv_.wait_for(lock,chrono::milliseconds(connectTimeout_),[&](){
        return !connectionQue_.empty();
    }))
    {
        LOG("获取连接超时");
        return nullptr;
    }


    //shared_ptr析构时会自动调用传入的删除器，这里传入的是lambda表达式，在连接放入队列时，重新绑定连接
    shared_ptr<Connection> sp (connectionQue_.front(),[&](Connection *p){
        unique_lock<mutex> lock(queMutex_);
        p->refershAliveTime();
        connectionQue_.push(p);
    });

    connectionQue_.pop();
    cv_.notify_all();

    return sp;
}

void ConnectionPool::produceConnectionTask()
{
    for(;;)
    {
        unique_lock<mutex> lock(queMutex_);

        //条件变量会检查条件，如果条件不满足，则线程进入等待状态，直到条件满足，才继续往下执行
        cv_.wait(lock,[&](){
            return connectionQue_.empty();
        });
        // while (!connectionQue_.empty()) 
        // {
        //     cv_.wait(lock);  //队列不空，此处生产线程进入等待状态(锁被释放)
        // }
        //连接数量没有到达上限，继续创建新的连接
        if(connectionCnt_ < maxSize_)
        {
            Connection *p = new Connection();
            p->connect(ip_,port_,userName_,passWd_,dbName_);
            p->refershAliveTime();
            connectionQue_.push(p);
            connectionCnt_++;
        }
        //通知消费线程，可以取连接了
        cv_.notify_all();
    }

}

void ConnectionPool::scannerConnectionTask()
{
    for(;;)
    {
        //扫描线程，每隔maxIdleTime_秒执行一次
        this_thread::sleep_for(chrono::seconds(maxIdleTime_));

        unique_lock<mutex> lock(queMutex_);
        //扫描整个队列，释放多余空闲时间连接
        while (connectionCnt_ > initSize_)
        {
            //顺序入队，队头时间未超过最大空闲时间，则其他的连接均未超过最大空闲时间
            Connection *p = connectionQue_.front(); 
            if(p->getAliveTime() >= (maxIdleTime_ * 1000))
            {
                connectionQue_.pop();
                connectionCnt_--;
                delete p;   //调用~Connection()释放连接
            }
            else
            {
                break;
            }
        }
    }
}