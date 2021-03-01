#ifndef RESPONSE_H  
#define RESPONSE_H

#include "Http.h"

class Response: public Http{
private:
    string status_code;
    string status_phrase;
    string protocol;
public:
    Response(){}
    string getProtocol() {return protocol;}
    string getStatusCode() {return status_code;}
    string getStatusPhrase() {return status_phrase;}
    virtual void parseFirstLine();
    ~Response(){}
};
#endif
