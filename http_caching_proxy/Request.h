#ifndef REQUEST_H
#define REQUEST_H
#include "Http.h"

using namespace std;

class Request: public Http{
private:
    int socket_fd;
    int u_id;
    string method;
    string protocol;
    string host;
    string port;
    string url;
public:
    Request::Request(int fd, int id):socket_fd(fd), u_id(id){}
    ~Request(){close(socket_fd);}
    Request(const Request & rhs);
    Request &operator=(const Request & rhs);
    string getMethod() {return method;}
    string getProtocol() {return protocol;}
    string getHost() {return host;}
    string getPort() {return port;}
    string getUrl() {return url;}
    int getSocket() {return socket_fd;}
    int getUid() {return u_id;}
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
