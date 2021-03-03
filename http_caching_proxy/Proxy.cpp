#include "Proxy.h"

//Not done
void proxy::runServer(){
  proxySocket.serverSetup();
  proxySocket.socketWaitConnect();
  int client_fd = proxySocket.socketAccept();
  Request * request = new Request(client_fd, request_id);// need modification #####
  request_id++;
  handler(request);// need to make this multi-thread #####
  delete request;
}

void proxy::handler(Request * request){
  const int buf_size = 4096;
  char buf[buf_size];// buffer to store recieved message from client
  int byte_count = recv(request->getSocket(), buf, buf_size - 1, 0);
  if (byte_count == -1) {// need to take care multi read? What if have zero? #####
    throw myException("Error recv.");
  }
  buf[byte_count] = '\0';
  std::string requestFull = buf;
  std::cout<<requestFull;
  request->parseHeader(requestFull);
  if (request->getMethod() == "GET"){
    handleGET(request);
  }
  else if (request->getMethod() == "POST"){
    handlePOST(request,requestFull);
  }
  else if (request->getMethod() == "CONNECT"){
    handleCONNECT(request);
  }
  else {// shouldn't happen
    // send 501?
  }
}

void proxy::handleGET(Request * request){
  return;
}

// need major update #####
void proxy::handlePOST(Request * request, std::string requestFull){
  int server_fd = runClient(request->getHost(), request->getPort());
  if (send(server_fd, requestFull.c_str(), requestFull.size() + 1, 0) == -1){
    throw myException("Error send.");
  }
  std::string received_data = receiveData(BUFFER_SIZE, server_fd);
  Response * response = new Response(server_fd, request_id);
  response->parseHeader(received_data);
  close(server_fd);
  /*
    Some logic to determine if response ok
  */
  sendData(received_data, request->getSocket());
  close(request->getSocket());
}

void proxy::handleCONNECT(Request * request){
  int server_fd = runClient(request->getHost(), request->getPort());
  //**** 200 OK reponse to client
  // need changes #####
  std::string OK_200("HTTP/1.1 200 Connection Established\r\n\r\n");
  if (send(request->getSocket(), OK_200.c_str(), OK_200.length(), 0) == -1) {
    perror("send 200 OK back failed");
  }
  fd_set readfds;
  struct timeval tv;
  tv.tv_sec = 2;
  while (true){
    char buf[BUFFER_SIZE];
    int len;
    FD_ZERO(&readfds);
    FD_SET(request->getSocket(), &readfds);
    FD_SET(server_fd, &readfds);
    if (select(FD_SETSIZE, &readfds, NULL, NULL, &tv) == 0){
      break;
    }
    else if (FD_ISSET(request->getSocket(), &readfds)) {
      len = recv(request->getSocket(), &buf, BUFFER_SIZE, 0);
      if (len < 0) {
        perror("Failed to recv from client in tunnel:");
        break;
      } 
      else if (len == 0) {
        break;
      }
      
      if (send(server_fd, buf, len, 0) < 0){
        perror("Failed to send to server in tunnel:");
        break;
      }
    }
    else if (FD_ISSET(server_fd, &readfds)){
      len = recv(server_fd, &buf, BUFFER_SIZE, 0);
      if (len < 0) {
        perror("Failed to recv from server in tunnel:");
        break;
      } 
      else if (len == 0) {
        break;
      }
      if (send(request->getSocket(), buf, len, 0) < 0) {
        perror("Failed to send to client in tunnel:");
        break;
      }
    }
    //buf.clear();
  }
  close(request->getSocket());
  close(server_fd);
}

// Not done
int proxy::runClient(std::string host, std::string port){
  socketInfo serverSocket = socketInfo(host.c_str(), port.c_str());
  serverSocket.clientSetup();
  serverSocket.socketConnect();
  return serverSocket.getFd();
}

void proxy::sendData(std::string data, int dest_fd){
  int bytes_total = data.size();
  int bytes_left = bytes_total;
  int bytes_sent = 0;
  int bytes_count;
  char *buf = new char[bytes_left];
  memcpy(buf, data.data(), bytes_left);

  while (bytes_sent < bytes_total) {
    if ((bytes_count = send(dest_fd, buf + bytes_sent, bytes_left, 0)) == -1) {
      throw myException("Error send");
    }    
    bytes_sent += bytes_count;
    bytes_left -= bytes_count;
  }
  delete [] buf;
}


std::string proxy::receiveData(int buf_size, int source_fd){
  std::string data = "";
  //int data_size = 0;
  while (1) {
    char buf[buf_size];
    int byte_count;
    if ((byte_count = recv(source_fd, buf, buf_size, 0)) == -1) {
      throw myException("Error recv");
    }
    // The server closed the connection
    if (byte_count == 0) {
      break;
    }  
    // Append the part of the response that was just received 
    data.append(buf, byte_count);
    //data_size += byte_count;
  }
  return data;
}