//
//  Http.h
//  HttpCachingProxy
//
//  Created by Richard on 2021/2/28.
//

#ifndef HTTP_H
#define HTTP_H
#include "myException.h"
#include "socket.h"
using namespace std;

class Http {
protected:
    unordered_map<string, string> headerPair;
    unordered_map<string, string> cacheControlPair;
    string firstLine;
    string headers;
    string body;
public:
    Http() {}
    void parseEachLine(string & msg) {
        size_t start = 0;
        if (msg.find(':', start) == string::npos) {
            throw myException("Invalid header");
        }
        while (start < msg.size()) {
            size_t colon = msg.find(':', start);
            size_t end = msg.find("\r\n", start);
            if (colon == string::npos) {
                throw myException("Invalid header");
            }
            string key = msg.substr(start, colon - start);
            string value = msg.substr(colon + 2, end - colon - 2);
            if (key.empty() || value.empty()) {
                throw myException("Invalid header");
            }
            headerPair[key] = value;
            start = end + 2;
        }
    }
    
    virtual void parseFirstLine() = 0;
    
    string getFirstLine() {
        return firstLine;
    }
    
    void parseHeader(string complete) {
        size_t endOfFirst = complete.find("\r\n", 0);
        size_t endOfRemain = complete.find("\r\n\r\n", endOfFirst);
        if (endOfFirst == string::npos || endOfRemain == string::npos) {
            throw myException("Invalid header");
        }
        this->firstLine = complete.substr(0, endOfFirst + 2);
        this->headers = complete.substr(endOfFirst + 2, endOfRemain - endOfFirst);
        parseEachLine(headers);
        parseFirstLine();
        parseCacheControl();
        int n;
        if ((n = getContentLenght()) != -1){
            this->body = complete.substr(endOfRemain + 4, n);
        }
        else {
            this->body = complete.substr(endOfRemain + 4);
        }
    }
    
    string getHeaderValue(string key) {
        string res = "";
        if (headerPair.find(key) != headerPair.end()){
            res = headerPair[key];
        }
        return res;
    }
    
    void addHeaderPair(string key, string value) {
        headerPair[key] = value;
    }
    
    int getContentLenght(){
        string n = getHeaderValue("Content-Length");
        if (n == ""){
            return -1;
        }
        return stoi(n);
    }

    bool checkIfChunked(){
        string encoding = getHeaderValue("Transfer-Encoding");
        if (encoding == "chunked"){
            return true;
        }
        return false;
    }

    void parseCacheControl() {
        string cacheControl;
        for (auto & [first, second] : headerPair) {
            if (first == "Cache-Control") {
                cacheControl = second;
                break;
            }
        }
        if (cacheControl.empty()) {
            return;
        }
        stringstream ss(cacheControl);
        string instruction;
        while (getline(ss, instruction, ',')) {
            // Eliminate all spaces in instruction
            instruction.erase(remove_if(instruction.begin(), instruction.end(), ::isspace), instruction.end());
            if (instruction.find('=') == string::npos) {
                cacheControlPair[instruction] = "";
            }
            else {
                size_t equal = instruction.find('=');
                string key = instruction.substr(0, equal);
                string value = instruction.substr(equal + 1);
                cacheControlPair[key] = value;
            }
        }
    }
    
    bool checkNoCacheInPragma() {
        for (auto & [first, second] : headerPair) {
            if (first == "Pragma" && second.find("no-cache") != string::npos) {
                return true;
            }
        }
        return false;
    }
    
    bool checkIfHeaderFieldExists(string str) {
        if (headerPair.find(str) == headerPair.end() || headerPair[str] == "") {
            return false;
        }
        return true;
    }
    
    bool checkCacheControlKey(string key) {
        for (auto & [first, second] : cacheControlPair) {
            if (first == key) {
                return true;
            }
        }
        return false;
    }
    
    string getCacheControlValue(string key) {
        for (auto & [first, second] : cacheControlPair) {
            if (first == key) {
                return second;
            }
        }
        throw myException("The key does not exist.");
    }
    
    void printPairs() {
        for (auto & [first, second] : headerPair) {
            cout << first << ": " << second << endl;
        }
    }
    void printCachePairs() {
        for (auto & [first, second] : cacheControlPair) {
            cout << first << ": " << second << endl;
        }
    }
    void printFirstLine() {
        cout << firstLine << endl;
    }
    
    string returnFirstLine() {
        return firstLine;
    }

    string getCompleteMessage(){
        return firstLine + headers + "\r\n" + body;
    }
};

#endif /* Http_h */
