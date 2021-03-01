#include "socket.h"
#define BACKLOG 100

socketInfo::socketInfo(const char * myPort):socket_fd(0), host_info_list(nullptr), hostname(nullptr), port(myPort){}

// do basic setup of the socket for server mode
void socketInfo::serverSetup(){
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  host_info.ai_socktype = SOCK_STREAM; // TCP stream sockets
  host_info.ai_flags = AI_PASSIVE;     // Autofill my IP
  int status;
  std::cout<<"Fetching address info(server)..."<<std::endl;
  if ((status = getaddrinfo(NULL, port, &host_info, &host_info_list)) != 0) {
    std::stringstream ss;
    ss << "Error getaddrinfo: " << gai_strerror(status);
    throw myException(ss.str());
  }
  std::cout<<"Creating a socket descriptor..."<<std::endl;
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
  std::cout<<"Fetching address info(client)..."<<std::endl;
  if ((status = getaddrinfo(NULL, port, &host_info, &host_info_list)) != 0) {
    std::stringstream ss;
    ss << "Error getaddrinfo: " << gai_strerror(status);
    throw myException(ss.str());
  }
  std::cout<<"Creating a socket descriptor..."<<std::endl;
  if ((socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol)) == -1){
    throw myException("Error socket.");
  }
}


// clear the port, bind socket with port and start waiting for connect request
void socketInfo::socketWaitConnect(){
  int yes = 1;
  std::cout<<"Clearing port..."<<std::endl;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    throw myException("Error setsockopt.");
  }
  int status;
  std::cout<<"Binding socket with port..."<<std::endl;
  if ((status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen)) == -1){
    throw myException("Error bind.");
  }
  std::cout<<"Listening for connections..."<<std::endl;
  if (listen(socket_fd, BACKLOG) == -1){
    throw myException("Error listen.");
  }
}

// accept request from given client, change its fd to the newly generated fd
void socketInfo::socketAccept(int & client_fd){
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  addr_size = sizeof(their_addr);
  std::cout<<"Accepting connection..."<<std::endl;
  client_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);
  if (client_fd == -1){
    throw myException("Error accept.");
  }
}

// make the connection through socket
void socketInfo::socketConnect(){
  std::cout<<"Initiating connection with socket..."<<std::endl;
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


