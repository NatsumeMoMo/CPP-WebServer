cmake_minimum_required(VERSION 3.10)
project(REACTOR)
set(CMAKE_CXX_STANDARD 20)
file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# set(SRCS
#     ${CMAKE_CURRENT_SOURCE_DIR}/TcpServer.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/HttpServer.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/HttpRequest.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/HttpResponse.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Connection.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Acceptor.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Buffer.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Channel.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Epoll.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/EventLoop.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/InetAddress.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Socket.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/Timestamp.cpp
#     # 其他源文件...
# )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(reactor ${SRCS})