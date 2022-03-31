#include "../Code/ProxyServer.h"
#include "../Code/Epoll.h"
#include <iostream>
#include <fstream>
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

int main(){
    ifstream fs;
    fs.open("req.txt");
    std::stringstream buffer;
    buffer << fs.rdbuf();
    cout << buffer.str() << endl;
    return 0;
}