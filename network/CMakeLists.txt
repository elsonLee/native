cmake_minimum_required(VERSION 3.10)

set(CXX_FLAGS
    -g
    -Wall
    -Werror
    -std=c++17
    )

find_package(Protobuf)
if(PROTOBUF_FOUND)
  message(STATUS "found protobuf")
endif()

set(SOURCES
    Acceptor.cpp
    Buffer.cpp
    Channel.cpp
    Connector.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    InetAddress.cpp
    Poller.cpp
    ProtobufCodec.cpp
    ProtobufDispatcher.cpp
    Socket.cpp
    SocketOps.cpp
    TcpClient.cpp
    TcpConnection.cpp
    TcpServer.cpp
    Timer.cpp
    TimerQueue.cpp
    )

add_library(network ${SOURCES})
target_link_libraries(network protobuf pthread)
install(TARGETS network DESTINATION ${PROJECT_BINARY_DIR}/lib)

set(HEADERS
    Acceptor.h
    Buffer.h
    Channel.h
    Common.h
    Connector.h
    EventLoop.h
    EventLoopThread.h
    Header.h
    InetAddress.h
    Poller.h
    ProtobufCodec.h
    ProtobufDispatcher.h
    Slice.h
    Socket.h
    SocketOps.h
    TcpClient.h
    TcpConnection.h
    TcpServer.h
    Timer.h
    TimerQueue.h
    Timestamp.h
    Utility.h
    )

install(FILES ${HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/include)
