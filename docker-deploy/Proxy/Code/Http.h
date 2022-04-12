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
    string pairs;
    int missing_body_length = -1;
    vector<char> headers;
    vector<char> body;
public:
    Http() {}
    void parsePairs(string & msg) {
        size_t start = 0;
        if (msg.find(':', start) == string::npos) {
            throw myException("Invalid header2");
        }
        while (start < msg.size()) {
            size_t colon = msg.find(':', start);
            size_t end = msg.find("\r\n", start);
            if (colon == string::npos) {
                throw myException("Invalid header3");
            }
            string key = msg.substr(start, colon - start);
            string value = msg.substr(colon + 2, end - colon - 2);
            if (key.empty() || value.empty()) {
                throw myException("Invalid header4");
            }
            headerPair[key] = value;
            start = end + 2;
        }
    }
    
    virtual void parseFirstLine() = 0;
    
    string getFirstLine() {
        return firstLine;
    }
    
    /*
        Only partially using vector of char to parse the HTTP message due to
        concern of re-use of old code. Could change the whole thing to parse 
        with vector of char.
    */
    void parseHeader(vector<char>& complete) {
        vector<char> endOfHeaderIndicator{'\r','\n','\r','\n'};
        auto startOfBody = std::search(complete.begin(), complete.end(), endOfHeaderIndicator.begin(), endOfHeaderIndicator.end())+4;
        this->headers = vector<char>(complete.begin(), startOfBody);
        string pairs_str(complete.begin(), startOfBody);
        size_t endOfFirst = pairs_str.find("\r\n", 0);
        size_t endOfRemain = pairs_str.find("\r\n\r\n", 0);
        if (endOfFirst == string::npos || endOfRemain == string::npos) {
            throw myException("Invalid header1");
        }
        this->firstLine = pairs_str.substr(0, endOfFirst + 2);
        this->pairs = pairs_str.substr(endOfFirst + 2, endOfRemain - endOfFirst);
        parsePairs(pairs);
        parseFirstLine();
        parseCacheControl();
        this->missing_body_length = getContentLenght();
        if (missing_body_length != -1){
            if (missing_body_length > complete.end()-startOfBody){ // haven't received full body
                this->body = vector<char>(startOfBody, complete.end());
                this->missing_body_length -= (complete.end()-startOfBody);
            }
            else {
                this->body = vector<char>(startOfBody, startOfBody+missing_body_length);
                this->missing_body_length = 0;
            }
        }
        else {
            vector<char> temp(startOfBody, complete.end());
            this->body = temp;
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

    int getBodySizeLeft(){
        return missing_body_length;
    }

    void addMissingBody(vector<char>& missing_body){
        if (missing_body.size()<missing_body_length){
            this->body.insert(this->body.end(), missing_body.begin(), missing_body.end());
            missing_body_length -= (missing_body.end()-missing_body.begin());
        }
        else {
            this->body.insert(this->body.end(), missing_body.begin(), missing_body.begin()+missing_body_length);
            missing_body_length = 0;
        }
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
    
    void printHeaders() {
        for (auto & [first, second] : headerPair) {
            cout << first << ": " << second << endl;
        }
    }
    void printCacheHeaders() {
        for (auto & [first, second] : cacheControlPair) {
            cout << first << ": " << second << endl;
        }
    }
    void printFirstLine() {
        cout << firstLine << endl;
    }

    vector<char> getCompleteMessage(){
        vector<char> complete;
        complete.reserve(headers.size()+body.size());
        complete.insert(complete.end(), headers.begin(), headers.end());
        complete.insert(complete.end(), body.begin(), body.end());
        return complete;
    }

    void printCompleteMessage(){
        for (char& c: getCompleteMessage()){
            cout << c;
        }
    }


};

#endif /* Http_h */
