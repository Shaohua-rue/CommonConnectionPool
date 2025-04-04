# CommonConnectionPool
## MySQL数据库连接池
### 关键技术点
MySQL数据库编程、单例模式、queue队列容器、C++11多线程编程、线程互斥、线程同步通信、unique_lock、基于CAS的原子整形、智能指针shared_ptr、lambda表达式、生产者-消费者模型
### 项目背景
为提高MySQL数据库(基于C/S设计)的访问瓶颈，除了在服务器端增加缓存服务器缓存常用的数据之外(如Redis)，还可以增加连接池，来提高MySQL数据库的访问效率。在高并发情况下，大量的**TCP三次握手、MySQL Server连接认证、MySQL Server关闭连接回收资源和TCP四次挥手**所耗费的性能时间也很明显，增加连接池即为了减少这一部分的性能消耗。

本项目基于C++11开发，提供MySQL Server的访问效率，实现基于C++代码的数据库连接池模块。

### 连接池功能点介绍
连接池一般包含了数据库连接所用的ip地址、port端口号、用户名和密码以及其它的性能参数，例如**初始连接量，最大连接量，最大空闲时间、连接超时时间**等，该项目是基于C++语言实现的连接池，主要是实现以上几个所有连接池都支持的通用基础功能。

**初始连接量(initSize)**:表示连接池事先会和MySQL Server创建initSize个数的connection连接，当应用发起MySQL访问时，不用再创建和MySQL Server新的连接，直接从连接池中获取一个可用的连接就可以，使用完成后，并不去释放connection，而是把当前connection再归还到连接池当中。

**最大连接量（maxSize）**：当并发访问MySQL Server的请求增多时，初始连接量已经不够使用了，此时会根据新的请求数量去创建更多的连接给应用去使用，但是新创建的连接数量上限是maxSize，不能无限制的创建连接，因为每一个连接都会占用一个socket资源，一般连接池和服务器程序是部署在一台主机上的，如果连接池占用过多的socket资源，那么服务器就不能接收太多的客户端请求了。当这些连接使用完成后，再次归还到连接池当中来维护。

**最大空闲时间（maxIdleTime）**：当访问MySQL的并发请求多了以后，连接池里面的连接数量会动态增加，上限是maxSize个，当这些连接用完再次归还到连接池当中。如果在指定的maxIdleTime里面，这些新增加的连接都没有被再次使用过，那么新增加的这些连接资源就要被回收掉，只需要保持初始连接量initSize个连接就可以了。

**连接超时时间（connectionTimeout）**：当MySQL的并发请求量过大，连接池中的连接数量已经到达maxSize了，而此时没有空闲的连接可供使用，那么此时应用从连接池获取连接无法成功，它通过阻塞的方式获取连接的时间如果超过connectionTimeout时间，那么获取连接失败，无法访问数据库。

该项目主要实现上述的连接池四大功能，其余连接池更多的扩展功能，可以自行实现。

## 功能实现设计
ConnectPool类：连接池代码实现
Connect类:数据库操作代码、增删改查代码实现
**连接池主要包含以下功能点：**
1.连接池只需要一个实例，所以ConnectPool类采用单例模式，保证连接池只有一个实例，并且线程安全（使用线程安全的懒汉式单例模式实现）;
2.ConnectPool中可以获取MySQL的连接Connection;
3.空闲连接Connection全部维护在一个线程安全的Connection队列中，使用线程互斥锁保证队列的线程安全
4.如果Connection队列为空，还需要再获取连接，此时需要动态创建连接，上限数量是maxSize;
5.队列中空闲连接时间超过maxIdleTime的就要被释放掉，只保留初始的initSize个连接就可以了，这个功能点肯定需要放在独立的线程中去做;
6.如果Connection队列为空，而此时连接的数量已达上限maxSize，那么等待connectionTimeout时间如果还获取不到空闲的连接，那么获取连接失败，此处从Connection队列获取空闲连接，可以使用带超时时间的mutex互斥锁来实现连接超时时间;
7.用户获取的连接用shared_ptr智能指针来管理，用lambda表达式定制连接释放的功能（不真正释放
连接，而是把连接归还到连接池中）;
8.连接的生产和连接的消费采用生产者-消费者线程模型来设计，使用了线程间的同步通信机制条件变量和互斥锁

## 压力测试

| 数据量  | 未使用连接池   | 使用连接池   |
| ------ | ------------ | ---------- |
| 1000   | 5109658ms    | 178795ms   |
| 5000   | 26299034ms   | 976597ms   |
| 10000  | 52702462ms   | 1459263ms  |

[参考博客]

[Ubuntu上安装MySQL详细指南](https://blog.csdn.net/qq_39071254/article/details/144617005?ops_request_misc=%257B%2522request%255Fid%2522%253A%252208aa90822989a4ab55c53c484c6fc8cd%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=08aa90822989a4ab55c53c484c6fc8cd&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~top_positive~default-2-144617005-null-null.142^v102^pc_search_result_base9&utm_term=ubuntu%E5%AE%89%E8%A3%85MYSQL&spm=1018.2226.3001.4187)

[MySQL Ubuntu下c++连接MySQL](https://blog.csdn.net/2201_75443644/article/details/144224665?ops_request_misc=%257B%2522request%255Fid%2522%253A%25222ac70fbd1f4e8d7fe751e2fc96eac306%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fall.%2522%257D&request_id=2ac70fbd1f4e8d7fe751e2fc96eac306&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_ecpm_v1~rank_v31_ecpm-10-144224665-null-null.142^v102^pc_search_result_base9&utm_term=ubuntu%E4%BD%BF%E7%94%A8MySQL%20%E5%BC%80%E5%8F%91%E5%8C%85%EF%BC%88%E5%8C%85%E6%8B%AC%E5%A4%B4%E6%96%87%E4%BB%B6%E5%92%8C%E5%8A%A8%E6%80%81%E5%BA%93%E6%96%87%E4%BB%B6%EF%BC%89&spm=1018.2226.3001.4187)

