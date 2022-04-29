#include "Cache.h"
mutex mtx;

bool Cache::checkIfUrlExists(string url) {
    for (auto & [first, second] : urlPair) {
        if (first == url) {
            return true;
        }
    }
    return false;
}

Response Cache::getCache(string url) {
    lock_guard<mutex> lck(mtx);
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
    LOG_INFO("(no-id): NOTE evicted %s from cache.", LRU.back())
    lock_guard<mutex> lck(mtx);
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
    double timeDiff = difftime(expireTime, time(0));
    return timeDiff > 0;
}

bool Cache::checkFreshness(string expireValue) {
    double timeDiff = difftime(strToTime(expireValue), time(0));
    return timeDiff > 0;
}

time_t Cache::strToTime(string str) {
    // Delete " GMT" in str
    size_t position = str.find(" GMT");
    if (position != string::npos) {
        str.erase(position, 4);
    }
    lock_guard<mutex> lck(mtx);
    const char* c = str.c_str();
    tm tmStruct;
    strptime(c, "%a, %d %b %Y %H:%M:%S" , &tmStruct);
    return mktime(&tmStruct);
}

bool Cache::validate(Request &request, Response &response) {
    string url = request.getUrl();
    if (checkIfUrlExists(url)) {
        int maxAge = INT_MAX;
        int maxStale = 0;
        if (request.checkNoCacheInPragma()) {
            addHeader(request, response);
            LOG_INFO("%d: NOTE Cached. Response will not be released from cache. Need revalidation", request.getUid());
            return false;
        }
        else {
            if (request.checkIfHeaderFieldExists("Cache-Control")) {
                if (request.checkCacheControlKey("no-cache")) {
                    addHeader(request, response);
                    LOG_INFO("%d: NOTE Cached. Response will not be released from cache. Need revalidation", request.getUid());
                    return false;
                }
                else if (request.checkCacheControlKey("no-store") || request.checkCacheControlKey("private")) {
                    LOG_INFO("%d: NOTE Response will not be stored in cache.(Request contains no-store/private)", request.getUid());
                    return false;
                }
                else {
                    if (request.checkCacheControlKey("max-age")) {
                        maxAge = stoi(request.getCacheControlValue("max-age"));
                        if (maxAge == 0) {
                            addHeader(request, response);
                            LOG_INFO("%d: NOTE Cached. Stale, need re-validation", request.getUid());
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
                    LOG_INFO("%d: NOTE Cached. Response will not be released from cache. Need revalidation", request.getUid());
                    return false;
                }
                else if (response.checkCacheControlKey("no-store") || response.checkCacheControlKey("private")) {
                    LOG_INFO("%d: NOTE Response will not be stored in cache.(Request contains no-store/private)", request.getUid());
                    return false;
                }
                else {
                    if (response.checkCacheControlKey("max-age")) {
                        maxAge = min(maxAge, stoi(response.getCacheControlValue("max-age")));
                        if (maxAge == 0) {
                            addHeader(request, response);
                            LOG_INFO("%d: NOTE Cached. Stale, need re-validation", request.getUid());
                            return false;
                        }
                    }
                    if (checkFreshness(maxAge, maxStale, response.getHeaderValue("Date"))) {
                        LOG_INFO("%d: In cache, valid", request.getUid());
                        LRUAdd(request.getUrl());
                        return true;
                    }
                    else {
                        addHeader(request, response);
                        LOG_INFO("%d: NOTE Cached. Stale, need re-validation.", request.getUid());
                        return false;
                    }
                }
            }
            else {
                if (response.checkIfHeaderFieldExists("Expires")) {
                    if (checkFreshness(response.getHeaderValue("Expires"))) {
                        LOG_INFO("%d: In cache, valid", request.getUid());
                        LRUAdd(request.getUrl());
                        return true;
                    }
                    else {
                        addHeader(request, response);
                        LOG_INFO("%d: NOTE Cached. Stale, need re-validation.", request.getUid());
                        return false;
                    }
                }
                else {
                    if (!checkFreshness(maxAge, maxStale, response.getHeaderValue("Date"))) {
                        addHeader(request, response);
                        LOG_INFO("%d: NOTE Cached. Stale, need re-validation.", request.getUid());
                        return false;
                    }
                    LOG_INFO("%d: In cache, valid", request.getUid());
                    LRUAdd(request.getUrl());
                    return true;
                }
            }
        }
    }
    LOG_INFO("%d: Not in cache", request.getUid());
    return false;
}

void Cache::addHeader(Request &request, Response &response) {
    if (response.checkIfHeaderFieldExists("Last-Modified")) {
        string LMVal = response.getHeaderValue("Last-Modified");
        LOG_INFO("%d: NOTE Last-Modified: %s", request.getUid(), LMVal);
        request.addHeaderPair("If-Modified-Since", LMVal);
    }
    if (response.checkIfHeaderFieldExists("ETag")) {
        string ETagVal = response.getHeaderValue("ETag");
        LOG_INFO("%d: NOTE NOTE ETag: %s", request.getUid(), ETagVal);
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
    lock_guard<mutex> lck(mtx);
    urlPair[url] = response;
    LRUAdd(url);
    if (LRU.size() > CACHESIZE) {
        LRUEvict();
    }
}


