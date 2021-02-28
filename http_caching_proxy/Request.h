#ifndef REQUEST_H
#define REQUEST_H
#include "Http.h"
#include "myException.h"
using namespace std;

class Request: public Http{
private:
    string method;
    string protocol;
    string host;
    string port;
    string url;
  
public:
    Request();
    ~Request();
    Request(const Request & rhs);
    Request &operator=(const Request & rhs);
    virtual void parseFirstLine();
    void printContents() {
        cout << "Method: " << method << endl;
        cout << "Protocol: " << protocol << endl;
        cout << "Host: " << host << endl;
        cout << "Port: " << port << endl;
        cout << "Url: " << url << endl;
    }
};
#endif
