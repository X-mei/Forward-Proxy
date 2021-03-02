#ifndef PROXY_H
#define PROXY_H
#include "socket.h"

class proxy{
private:
  socketInfo serverSocket;
  std::string port;
public:
  proxy(const char * portNum):port(portNum), serverSocket(portNum){}
  void runServer();
  ~proxy();
};
#endif