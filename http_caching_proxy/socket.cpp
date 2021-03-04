#include "socket.h"
#define BACKLOG 100

socketInfo::socketInfo(const char * myHost, const char * myPort):socket_fd(0), host_info_list(nullptr), hostname(myHost), port(myPort){}

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
  if ((status = getaddrinfo(hostname, port, &host_info, &host_info_list)) != 0) {
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

// accept request from given client, return the newly generated fd
int socketInfo::socketAccept(int client_id){
  struct sockaddr_storage their_addr;
  //int MAXDATASIZE = 8000;
  //char buffer[MAXDATASIZE];
  char s[INET6_ADDRSTRLEN];
  socklen_t addr_size;
  addr_size = sizeof(their_addr);
  std::cout<<"Accepting connection..."<<std::endl;
  int client_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);
  if (client_fd == -1){
    throw myException("Error accept.");
  }
  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
  printf("server: got connection from %s\n", s);
  //proxy_log << ;
  /*
  if (recv(client_fd, buffer, sizeof(buffer), 0) == -1) {
    throw myException("Error recv.");
  }
  //std::cout<<"recieved:\n"<<buffer<<std::endl;
  */
  return client_fd;
}

// make the connection through socket
void socketInfo::socketConnect(){
  std::cout<<"Initiating connection with socket..."<<std::endl;
  if (connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) == -1){
    close(socket_fd);
    std::cout<<"Error connect.";
    throw myException("Error connect.");
  }
  /*
  std::cout<<"Still good"<<std::endl;
  if (send(socket_fd, "Hello, world!", 13, 0) == -1){
    perror("Error send.");
  }
  */         
}

// socketInfo destructor
socketInfo::~socketInfo(){
  if (socket_fd == 0){
    close(socket_fd);
  }
  if (host_info_list != nullptr){
    free(host_info_list);
  }
}

// helper, get sockaddr, IPv4 or IPv6:
void * socketInfo::get_in_addr(struct sockaddr *sa){
  if (sa->sa_family == AF_INET){
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


