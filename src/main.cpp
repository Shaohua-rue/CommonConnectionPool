#include <iostream>
#include "Connection.h"
int main()
{
    MYSQL * my = mysql_init(nullptr);
    std::cout << mysql_get_client_info() << std::endl;

    Connection conn;
    conn.connect("127.0.0.1",3306,"shaohua","010407","chat");
    if(conn.update("insert into user(id,name,age,sex) values('1','123456','123','male');"))
    {
        std::cout << "insert success" << std::endl;
    }
    else
    {
        std::cout << "insert fail" << std::endl;
    }
    return 0;
}