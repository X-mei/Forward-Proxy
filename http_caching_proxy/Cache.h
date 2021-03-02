#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <list>
#include "myException.h"
#include "Response.h"
#include "Request.h"

using namespace std;

class Cache {
//public:
private:
    list<string> LRU;
    unordered_map<string, Response> urlPair;
    bool checkIfCacheExists(string url);
    Response getCache(string url);
    void LRUEvict();
    void LRUAdd(string url);
    void printLRU() {
        for (list<string>::iterator it = LRU.begin(); it != LRU.end(); ++it) {
            cout << *it << endl;
        }
    }
public:
    bool validateCache(Request & request, Response & response, string & msg);
    void handle(Request & request, Response & response, string & msg);
    
};
