#include "../Code/ProxyServer.h"
#include "../Code/Epoll.h"
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

int main(){
    ProxyServer svr(170,4,23456,false,-1,1024);//use default trigger mode and log level, disable log
    svr.RunServer();
    return 0;
}