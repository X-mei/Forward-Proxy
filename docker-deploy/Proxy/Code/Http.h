//
//  Http.h
//  HttpCachingProxy
//
//  Created by Richard on 2021/2/28.
//

#ifndef HTTP_H
#define HTTP_H
#include "common.h"
#include "socket.h"
#include "myException.h"

using namespace std;

class Http {
protected:
    unordered_map<string, string> headerPair;
    unordered_map<string, string> cacheControlPair;
    string firstLine;
    string pairs;
    int missing_body_length = -1;
    vector<char> headers;
    vector<char> body;
public:
    Http() {}
    ~Http() {}
    void parsePairs(string & msg);
    
    virtual void parseFirstLine() = 0;
    
    string getFirstLine();
    
    /*
        Only partially using vector of char to parse the HTTP message due to
        concern of re-use of old code. Could change the whole thing to parse 
        with vector of char.
    */
    void parseHeader(vector<char>& complete);
    
    string getHeaderValue(string key);
    
    void addHeaderPair(string key, string value);
    
    int getContentLenght();

    bool checkIfChunked();

    int getBodySizeLeft();

    void addMissingBody(vector<char>& missing_body);

    void parseCacheControl();
    
    bool checkNoCacheInPragma();
    
    bool checkIfHeaderFieldExists(string str);
    
    bool checkCacheControlKey(string key);
    
    string getCacheControlValue(string key);
    
    void printHeaders();
    
    void printCacheHeaders();

    void printFirstLine();

    vector<char> getCompleteMessage();

    void printCompleteMessage();
};

#endif /* Http_h */
