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
#include "Proxy.h"
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

void testProxy(const char * port){
    proxy * p = new proxy(port);
    p->runServer();
    //p->runClient();
}

int main(int argc, const char * argv[]) {
    const char * port = "12345";
    //testRequestParser();
    testProxy(port);
    return 0;
}
