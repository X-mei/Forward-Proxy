#include "../Code/ProxyServer.h"
#include <time.h>
#include <thread>

using namespace std;

int main(){
    ProxyServer svr(170,4,12345,true,0,1024);//use default trigger mode and log level, disable log
    svr.RunServer();
    return 0;
}