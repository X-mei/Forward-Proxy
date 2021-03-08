#ifndef CACHE_H
#define CACHE_H
#include "common.h"
#include "myException.h"
#include "Response.h"
#include "Request.h"
#include "Log.h"

#define CACHESIZE 100

using namespace std;

class Cache {
//public:
private:
    list<string> LRU;
    unordered_map<string, Response> urlPair;
    Log* log;
    bool checkIfUrlExists(string url);
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
    Cache() {
        log = new Log;
    }
    bool validate(Request & request, Response & response);
    void handle(Request & request, Response & response);
    Response getCache(string url);
};
#endif