//
//  Http.h
//  HttpCachingProxy
//
//  Created by Richard on 2021/2/28.
//

#ifndef HTTP_H
#define HTTP_H
#include <unordered_map>
#include <string>
#include <iostream>
#include <myException.h>
using namespace std;

class Http {
protected:
    unordered_map<string, string> headerPair;
    string firstLine;
public:
    Http() {}
    void parseEachLine(string & msg) {
        size_t start = 0;
        if (msg.find(':', start) == string::npos) {
            cout <<"Invalid header"<< endl;
            //throw MyException("Invalid header");
        }
        while (start < msg.size()) {
            size_t colon = msg.find(':', start);
            size_t end = msg.find("\r\n", start);
            if (colon == string::npos) {
                cout <<"Invalid header"<< endl;
                //throw MyException("Invalid header");
            }
            string key = msg.substr(start, colon - start);
            string value = msg.substr(colon + 2, end - colon - 2);
            if (key.empty() || value.empty()) {
                cout <<"Invalid header"<< endl;
                //throw MyException("Invalid header");
            }
            headerPair[key] = value;
            start= end + 2;
        }
    }
    
    virtual void parseFirstLine() = 0;
    
    string getFirstLine() {
        return firstLine;
    }
    void parseHeader(string & msg) {
        size_t endOfFirst = msg.find("\r\n", 0);
        size_t endOfRemain = msg.find("\r\n\r\n", endOfFirst);
        if (endOfFirst == string::npos || endOfRemain == string::npos) {
            cout <<"Invalid header"<< endl;
            //throw MyException("Invalid header");
        }
        this->firstLine = msg.substr(0, endOfFirst);
        string remainHeader = msg.substr(endOfFirst + 2, endOfRemain - endOfFirst);
        parseEachLine(remainHeader);
        parseFirstLine();
    }
    
    void printPairs() {
        for (auto & [first, second] : headerPair) {
            cout << first << ": " << second << endl;
        }
    }
    void printFirstLine() {
        cout << firstLine << endl;
    }
};

#endif /* Http_h */
