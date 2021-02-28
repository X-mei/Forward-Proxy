#include "socket.h"
#define BACKLOG 10

socketInfo::socketInfo():socket_fd(0), host_info_list(nullptr), hostname(nullptr), port(nullptr){}

// do basic setup of the socket for server mode
void socketInfo::serverSetup(){
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  host_info.ai_socktype = SOCK_STREAM; // TCP stream sockets
  host_info.ai_flags = AI_PASSIVE;     // Autofill my IP
  int status;
  if ((status = getaddrinfo(NULL, port, &host_info, &host_info_list)) != 0) {
    std::stringstream ss;
    ss << "Error getaddrinfo: " << gai_strerror(status);
    throw myException(ss.str());
  }
  if ((socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol)) == -1){
    throw myException("Error socket.");
  }
}

// do basic setup of the socket for client mode
void socketInfo::clientSetup(){
  memset(&host_info, 0, sizeof(host_info)); // make sure the struct is empty
  host_info.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  host_info.ai_socktype = SOCK_STREAM; // TCP stream sockets
  int status;
  if ((status = getaddrinfo(NULL, port, &host_info, &host_info_list)) != 0) {
    std::stringstream ss;
    ss << "Error getaddrinfo: " << gai_strerror(status);
    throw myException(ss.str());
  }
  if ((socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol)) == -1){
    throw myException("Error socket.");
  }
}


// clear the port, bind socket with port and start waiting for connect request
void socketInfo::socketWaitConnect(){
  int yes = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    throw myException("Error setsockopt.");
  }
  int status;
  if ((status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen)) == -1){
    throw myException("Error bind.");
  }
  if (listen(socket_fd, BACKLOG) == -1){
    throw myException("Error listen.");
  }
}

// accept request from given client, change its fd to the newly generated fd
void socketInfo::socketAccept(int & client_fd){
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  addr_size = sizeof(their_addr);
  client_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);
  if (client_fd == -1){
    throw myException("Error accept.");
  }
}

// make the connection through socket
void socketInfo::socketConnect(){
  if (connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) == -1){
    throw myException("Error connect.");
  }
}

socketInfo::~socketInfo(){
  if (socket_fd == 0){
    close(socket_fd);
  }
  if (host_info_list != nullptr){
    free(host_info_list);
  }
}


