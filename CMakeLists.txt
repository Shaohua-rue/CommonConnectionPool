cmake_minimum_required(VERSION 3.0)
project(CommonConnectivityTest)
include_directories(
    include
)
add_executable(main 
src/main.cpp
src/Connection.cpp
src/CommonConnectionPool.cpp)
target_link_libraries(main -lmysqlclient)