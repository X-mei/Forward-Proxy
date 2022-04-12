#ifndef RESPONSE_H  
#define RESPONSE_H

#include "common.h"
#include "Http.h"

class Response: public Http{
private:
  int socket_fd;
  int u_id;
  string status_code;
  string status_phrase;
  string protocol;
public:
  Response();
  Response(int fd, int id):socket_fd(fd), u_id(id){}
  string getProtocol() {return protocol;}
  string getStatusCode() {return status_code;}
  string getStatusPhrase() {return status_phrase;}
  virtual void parseFirstLine();
  int getSocket() {return socket_fd;}
  int getUid() {return u_id;}
  ~Response();
};
#endif
