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
  int byte_count = recv(request->socket_fd, buf, buf_size - 1, 0);
  if (byte_count == -1) {// need to take care multi read? What if have zero? #####
    throw myException("Error recv.");
  }
  buf[byte_count] = '\0';
  request->pareseHeader(std::string(buf));
  if (request->getMethod() == "GET"){
    handleGET(request);
  }
  else if (request->getMethod() == "POST"){
    handlePOST(request);
  }
  else {// CONNECT
    handleCONNECT(request);
  }
}

// need major update #####
void proxy::handlePOST(Request * request){
  int server_fd = runClient(request->getHost(), request->getPort());
  if (send(server_fd, new_request_buf, strlen(new_request_buf) + 1, 0) == -1){
    throw myException("Error send.");
  }
  std::string recieved_data = receive_data();// need implement #####
  Response * response = new Response(server_fd, request_id);
  Response->parseHeader(recieved_data);
  close(server_fd);
  /*
    Some logic to determine if response ok
  */
  send_data();// need implement #####
  close(request->getSocket());
}

// Not done
int proxy::runClient(std::string host, std::string port){
  socketInfo serverSocket = socketInfo(host.c_str(), port.c_str());
  proxySocket.clientSetup();
  proxySocket.socketConnect();
  return serverSocket->getFd();
}