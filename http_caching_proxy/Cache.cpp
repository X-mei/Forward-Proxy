#include "Cache.h"

bool Cache::checkIfUrlExists(string url) {
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
    urlPair.erase(LRU.back());
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

bool Cache::checkFreshness(int maxAge, int maxStale, string dateValue) {
    time_t expireTime = maxAge + maxStale + strToTime(dateValue);
    double timeDiff = difftime(time(0), expireTime);
    return timeDiff > 0;
}

bool Cache::checkFreshness(string expireValue) {
    double timeDiff = difftime(time(0), strToTime(expireValue));
    return timeDiff > 0;
}

time_t Cache::strToTime(string str) {
    const char* c = str.c_str();
    tm* tmStruct = nullptr;
    strptime(c, "%a, %d %b %Y %H:%M:%S" , tmStruct);
    return mktime(tmStruct);
}

bool Cache::validate(Request &request, Response &response) {
    string url = request.getUrl();
    if (checkIfUrlExists(url)) {
        int maxAge = INT_MAX;
        int maxStale = 0;
        if (request.checkNoCacheInPragma()) {
            addHeader(request, response);
            cout << "Response will not be released from cache. Need revalidation." << endl;
            return false;
        }
        else {
            if (request.checkIfHeaderFieldExists("Cache-Control")) {
                if (request.checkCacheControlKey("no-cache")) {
                    addHeader(request, response);
                    cout << "Response will not be released from cache. Need revalidation." << endl;
                    return false;
                }
                else if (request.checkCacheControlKey("no-store") || request.checkCacheControlKey("private")) {
                    cout << "Response will not be stored in cache.(Request contains no-store/private)" << endl;
                    return false;
                }
                else {
                    if (request.checkCacheControlKey("max-age")) {
                        maxAge = stoi(request.getCacheControlValue("max-age"));
                        if (maxAge == 0) {
                            addHeader(request, response);
                            cout << "request max-age is 0, validation is false." << endl;
                            return false;
                        }
                    }
                    if (request.checkCacheControlKey("max-stale")) {
                        maxStale = stoi(request.getCacheControlValue("max-stale"));
                    }
                }
            }
            if (response.checkIfHeaderFieldExists("Cache-Control")) {
                if (response.checkCacheControlKey("no-cache")) {
                    addHeader(request, response);
                    cout << "Response will not be released from cache. Need revalidation." << endl;
                    return false;
                }
                else if (response.checkCacheControlKey("no-store") || response.checkCacheControlKey("private")) {
                    cout << "Response will not be stored in cache.(Response contains no-store/private)" << endl;
                    return false;
                }
                else {
                    if (response.checkCacheControlKey("max-age")) {
                        maxAge = min(maxAge, stoi(response.getCacheControlValue("max-age")));
                        if (maxAge == 0) {
                            addHeader(request, response);
                            cout << "response max-age is 0, validation is false." << endl;
                            return false;
                        }
                    }
                    if (checkFreshness(maxAge, maxStale, response.getHeaderValue("Date"))) {
                        cout << "response is fresh, validation is true." << endl;
                        cout << "Cache updated." << endl;
                        LRUAdd(request.getUrl());
                        return true;
                    }
                    else {
                        addHeader(request, response);
                        cout << "Response is not fresh, validation is false." << endl;
                        return false;
                    }
                }
            }
            else {
                if (response.checkIfHeaderFieldExists("Expires")) {
                    if (checkFreshness(response.getHeaderValue("Expires"))) {
                        cout << "Not pass expire time, validation is true." << endl;
                        cout << "Cache updated." << endl;
                        LRUAdd(request.getUrl());
                        return true;
                    }
                    else {
                        addHeader(request, response);
                        cout << "Pass expire time, validation is false." << endl;
                        return false;
                    }
                }
                else {
                    cout << "No cache control, no expire, response is in cache, validation is true." << endl;
                    cout << "Cache updated." << endl;
                    LRUAdd(request.getUrl());
                    return true;
                }
            }
        }
    }
    cout << "Url is not in the cache." << endl;
    return false;
}

void Cache::addHeader(Request &request, Response &response) {
    if (response.checkIfHeaderFieldExists("Last-Modified")) {
        string LMVal = response.getHeaderValue("Last-Modified");
        request.addHeaderPair("If-Modified-Since", LMVal);
    }
    if (response.checkIfHeaderFieldExists("ETag")) {
        string ETagVal = response.getHeaderValue("ETag");
        request.addHeaderPair("If-None-Match", ETagVal);
    }
    cout << "Validation header added." << endl;
}

void Cache::handle(Request & request, Response & response) {
    if (response.getStatusCode() == "304") {
        cout << "Response status is 304, response is aquired from cache." << endl;
        response = getCache(request.getUrl());
    }
    else {
        if (request.checkIfHeaderFieldExists("Cache-Control") && request.checkCacheControlKey("no-store")) {
            cout << "Request contains no-store, not stored in cache." << endl;
            return;
        }
        if (response.checkIfHeaderFieldExists("Cache-Control")) {
            if (response.checkCacheControlKey("no-store") || response.checkCacheControlKey("private")) {
                cout << "Response contains no-store or private, not stored in cache." << endl;
                return;
            }
        }
    }
    cout << "Cache stored." << endl;
    updateCache(request.getUrl(), response);
}

void Cache::updateCache(string url, Response response) {
    urlPair[url] = response;
    LRUAdd(url);
    if (LRU.size() > CACHESIZE) {
        LRUEvict();
    }
}
