#ifndef PROXY_H
#define PROXY_H
#include "socket.h"

class proxy{
private:
  socketInfo proxySocket;
  int request_id;
  std::string port;
  void handlePOST(Request* request);
  void handleGET(Request* request);
  void handleCONNECT(Request* request);
public:
  proxy(const char * portNum):port(portNum), proxySocket(portNum), request_id(1){}
  void runServer();
  int runClient();
  void handler(Request* request);
  ~proxy();
};
#endif
