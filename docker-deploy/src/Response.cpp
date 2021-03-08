#include "Response.h"

Response::~Response(){}

Response::Response(){}

void Response::parseFirstLine(){
    size_t space1 = firstLine.find(' ');
    if (space1 == string::npos){
        throw myException("Error in first line syntax.");
    }
    string temp = firstLine.substr(0, space1);
    this->protocol = temp;
    size_t space2 = firstLine.find(' ');
    if (space2 == string::npos){
        throw myException("Error in first line syntax.");
    }
    temp = firstLine.substr(space1+1, space2-space1-1);
    this->status_code = temp;
    this->status_phrase = firstLine.substr(space2+1);
}

string Response::getContents() {
    string header = firstLine + "\r\n";
    // header + body
    for (auto & [first, second] : headerPair) {
        header += first + ": " + second + "\r\n";
    }
    return header + "\r\n\r\n" + body;
}
