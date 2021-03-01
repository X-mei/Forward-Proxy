#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include "myException.h"
class socketInfo {
private:
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname;
  const char *port;
public:
  socketInfo(const char * myPort);
  void serverSetup();
  void clientSetup();
  void socketWaitConnect();
  void socketAccept(int & client_fd);
  void socketConnect();
  ~socketInfo();
};
#endif
