#include "../ProxyServer.h"
#include "../Epoll.h"
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

int main(){
    ProxyServer svr(10,10,4,32323);//use default trigger mode
    svr.RunServer();
    return 0;
}