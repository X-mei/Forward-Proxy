#ifndef MYSOCKET_H
#define MYSOCKET_H

#include "common.h"
#include "myException.h"
class socketInfo {
private:
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname;
  const char *port;
  // get sockaddr, IPv4 or IPv6:
  void *get_in_addr(struct sockaddr *sa);
public:
  socketInfo(const char * myHost, const char * myPort);
  void serverSetup();
  void clientSetup();
  void socketWaitConnect();
  void socketAccept(int & client_fd, std::string& ip_address);
  void socketConnect();
  int getFd(){return socket_fd;}
  ~socketInfo();
};
#endif
