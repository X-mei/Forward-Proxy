#include "Request.h"
Request::Request(){}

void Request::parseFirstLine(){
  string firstLine = getFirstLine();
  // get method 
  size_t space1 = firstLine.find(' ');
  if (space1 == string::npos){
    throw myException("Error in first line syntax.");
  }
  string temp = firstLine.substr(0, space1);//need to store this
  if (temp != "GET" && temp != "POST" && temp != "CONNECT"){
    throw myException("Method not suppoted.");
  }
  this->method = temp;
  // get protocol
  size_t http = firstLine.find("http");
  temp = "http";//need to store this
  if (http && (http+4)<firstLine.size() && firstLine[http+4]=='s'){
    temp = "https";
  }
  if (this->method == "CONNECT"){
    temp = "https";
  }
  this->protocol = temp;
  // get port and host
  size_t space2 = firstLine.find(' ', space1+1);
  if (space2 == string::npos){
    throw myException("Error in first line syntax.");
  }
  temp = firstLine.substr(space1+1, space2-space1-1);
  if (http != string::npos){//absolute form
    size_t slash = firstLine.find('/');
    temp = firstLine.substr(slash+2, space2-slash-2);//-3?
    size_t slash_middle = temp.find('/');
    size_t colon = temp.find(':');
    if (colon != string::npos){
      if (slash_middle != string::npos){
        this->port = temp.substr(colon+1, slash_middle-colon-1);
        this->host = temp.substr(0, slash_middle);
      }
    }
  }
  else {//authority form
    size_t colon = temp.find(':');
    if (colon != string::npos){
      this->port = temp.substr(colon+1);
      this->host = temp.substr(0, colon);
    }
  }
  if (this->port.empty()){//if origin form
    this->port = (this->protocol == "http")?"80":"443";
  }
  if (this->host.empty()){//if origin form
    this->host = headerPair["Host"];
  }
  //get url pending
}
