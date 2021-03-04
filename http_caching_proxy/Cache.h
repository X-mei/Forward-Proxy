#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <list>
#include <ctime>
#include "myException.h"
#include "Response.h"
#include "Request.h"

#define CACHESIZE 100

using namespace std;

class Cache {
//public:
private:
    list<string> LRU;
    unordered_map<string, Response> urlPair;
    bool checkIfUrlExists(string url);
    Response getCache(string url);
    void LRUEvict();
    void LRUAdd(string url);
    bool checkFreshness(int maxAge, int maxStale, string dateValue);
    bool checkFreshness(string expireValue);
    time_t strToTime(string str);
    void addHeader(Request &request, Response &response);
    void updateCache(string url, Response response);
    void printLRU() {
        for (list<string>::iterator it = LRU.begin(); it != LRU.end(); ++it) {
            cout << *it << endl;
        }
    }
    
public:
    bool validate(Request & request, Response & response);
    void handle(Request & request, Response & response);
    
};
