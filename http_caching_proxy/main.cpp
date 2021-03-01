//
//  main.cpp
//  HttpCachingProxy
//
//  Created by Richard on 2021/2/28.
//

#include <iostream>
#include "Http.h"
#include "socket.h"
#include "Response.h"
#include "Request.h"
#include "Cache.h"
#include "myException.h"

void testRequestParser(){
    Request * h = new Request();
    string msg = "CONNECT http://www.example.org:50/pub/WWW/TheProject.html HTTP/1.1\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 51\r\nVary: Accept-Encoding\r\nCache-Control: public, max-age=1234, no-cache\r\nPragma: no-cache\r\nContent-Type: text/plain\r\n\r\n";
    h->parseHeader(msg);
    h->printPairs();
    h->printFirstLine();
    h->printContents();
    h->printCachePairs();
    if (h->checkNoCacheInPragma() == true) {
        cout << "true" << endl;
    }
    if (h->checkCacheControlKey("no-cache")) {
        cout << "true" << endl;
    }
    if (h->checkIfHeaderFieldExists("Cache-Control")) {
        cout << "true" << endl;
    }
}

void testCache() {
    Cache *c = new Cache;
    /*
    string s1 = "str1";
    string s2 = "str2";
    string s3 = "str3";
    string s4 = "str4";
    string s5 = "str5";
    string s6 = "str6";
    c->LRUAdd(s1);
    c->LRUAdd(s2);
    c->LRUAdd(s3);
    c->LRUAdd(s4);
    c->LRUAdd(s5);
    c->LRUAdd(s6);
    c->printLRU();
    cout << "========" << endl;
    c->LRUEvict();
    c->printLRU();
    cout << "========" << endl;
    c->LRUEvict();
    c->printLRU();
    cout << "========" << endl;
    c->LRUAdd(s1);
    c->printLRU();
     */
}

int main(int argc, const char * argv[]) {
    //testRequestParser();
    testCache();
    return 0;
}
