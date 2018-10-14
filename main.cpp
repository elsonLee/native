#include "Common.h"

#include "Channel.h"
#include "EventLoop.h"
#include "SocketOps.h"
#include "Acceptor.h"

#include <sys/timerfd.h>
#include <iostream>
#include <thread>

EventLoop* g_loop = nullptr;

void
timeout ()
{
    std::cout << "Timeout!" << std::endl;
    //g_loop->quit();
}

void
func_cb ()
{
    std::cout << "Timeout!" << std::endl;
    g_loop->quit();
}

void
newConnection (int sockfd, const InetAddress& addr)
{
    std::cout << "newConnection: accept a connection from " << std::endl;
    ::write(sockfd, "How are you?\n", 13);
    sockops::close(sockfd);
}

int main ()
{
#if 0
    EventLoop loop;
    //g_loop = &loop;

    //int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    //Channel ch(timerFd, &loop);
    //ch.setReadCallback(timeout);
    //ch.enableReadEvent();

    //struct itimerspec howlong;
    //bzero(&howlong, sizeof(howlong));
    //howlong.it_value.tv_sec = 5;
    //::timerfd_settime(timerFd, 0, &howlong, NULL);

    //std::thread thr([&](){ loop.runAfter(5000000, [](){ std::cout << "Test Timeout!" << std::endl;}); });
    //std::thread thr_1([&](){ loop.runEvery(3000000, [](){ std::cout << "Test Timeout!" << std::endl;}); });
    std::thread thr_1([&](){
            usleep(1000000);
            loop.runInLoop([](){ std::cout << "Test Funcion in IO thread1!" << std::endl;});
    });

    std::thread thr_2([&](){
            usleep(1000000);
            loop.runInLoop([&](){ loop.quit(); std::cout << "Test Funcion in IO thread2!" << std::endl;});
    });

    loop.run();

    //::close(timerFd);

    thr_1.join();
    thr_2.join();
#endif

    InetAddress listenAddr(9981);
    EventLoop loop;

    Acceptor acceptor(&loop, listenAddr);
    acceptor.setConnectionCallback(newConnection);
    acceptor.listen();

    loop.run();

    return 0;
}
