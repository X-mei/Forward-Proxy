#include "Cache.h"

bool Cache::checkIfCacheExists(string url) {
    for (auto & [first, second] : urlPair) {
        if (first == url) {
            return true;
        }
    }
    return false;
}

Response Cache::getCache(string url) {
    for (auto & [first, second] : urlPair) {
        if (first == url) {
            return second;
        }
    }
    string message = "No such url found.";
    cout << message << endl;
    Response r;
    return r;
}

void Cache::LRUEvict() {
    if (LRU.empty()) {
        return;
    }
    urlPair.erase(LRU.front());
    LRU.pop_back();
}

void Cache::LRUAdd(string url) {
    for (list<string>::iterator it = LRU.begin(); it != LRU.end(); ++it) {
        if (*it == url) {
            LRU.erase(it);
            break;
        }
    }
    LRU.push_front(url);
}
