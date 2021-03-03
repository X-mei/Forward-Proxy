#ifndef PROXY_H
#define PROXY_H
#include <thread>
#include "socket.h"
#include "Request.h"
#include "Response.h"
#define BUFFER_SIZE 20000
class proxy{
private:
  socketInfo proxySocket;
  int request_id;
  std::string port;
  void handlePOST(Request* request, std::string requestFull);
  void handleGET(Request* request);
  void handleCONNECT(Request* request);
  void sendData(std::string data, int dest_fd);
  std::string receiveData(int buf_size, int source_fd);
public:
  proxy(const char * portNum):port(portNum), proxySocket(nullptr, portNum), request_id(1){}
  void runServer();
  int runClient(std::string host, std::string port);
  void handler(Request* request);
  ~proxy();
};
#endif
