#include "../Code/ProxyServer.h"
#include "../Code/Epoll.h"
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

int main(){
    ProxyServer svr(10,4,32323,true,-1,1024);//use default trigger mode and log level
    svr.RunServer();
    return 0;
}