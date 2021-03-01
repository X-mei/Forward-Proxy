#include "Response.h"


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
