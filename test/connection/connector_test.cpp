#include "Header.h"

void
newConnectionCallback (int sockfd) {
    printf("hello world\n");
}

int main () {
    EventLoopThread t;
    auto loop = t.start();
    Connector connector(loop, InetAddress(5000));
    connector.setConnectCallback(newConnectionCallback);

    connector.start();

    sleep(10);
};
