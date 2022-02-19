#include "../Code/ProxyServer.h"
#include "../Code/Epoll.h"
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

int main(){
    vector<char> v;
    string str;
    cout << v.max_size() << endl;
    cout << str.max_size() << endl;
    //ProxyServer svr(10,4,12345,true,-1,1024);//use default trigger mode and log level
    //svr.RunServer();
    return 0;
}