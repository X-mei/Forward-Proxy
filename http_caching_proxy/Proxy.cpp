#include "Proxy.h"
mutex mtx;

// Server init and connection with client exception handled at this level
void proxy::runServer(){
  mtx.lock();
  try{
    proxySocket.serverSetup();
    proxySocket.socketWaitConnect();
  }
  catch(myException e){
    //std::cout<<e.what();
    return;
  }
  // Run forever, take request and handle it by branching a new thread
  while(true){
    int client_fd;
    std::string ip_address;
    try{
      proxySocket.socketAccept(client_fd, ip_address);
    }
    catch(myException e){
      //std::cout<<e.what();
      continue;
    }
    Request * request = new Request(client_fd, request_id);
    std::cout<<"######"<<request_id<<"######\n";
    // Logging related action
    std::time_t seconds = std::time(nullptr);
    std::string request_time = std::string(std::asctime(std::gmtime(&seconds)));
    request_time = request_time.substr(0, request_time.find("\n"));
    
    request_id++;
    
    std::thread trd(&proxy::handler, this, request, ip_address, request_time);// Branching a new thread, handle it
    trd.detach();// Detach the thread from the main thread
  }
  mtx.unlock();
}

// This functions handles the request based on its method
// Request error is handled at this level
void proxy::handler(Request * request, string ip_address, string request_time){
  const int buf_size = 4096;
  char buf[buf_size];// buffer to store recieved message from client
  int byte_count = recv(request->getSocket(), buf, buf_size - 1, 0);
  try{
    if (byte_count == -1) {
      throw myException("Error recv.");
    }
    if (byte_count == 0){
      throw myException("Clint closed connection.");
    }
    buf[byte_count] = '\0';
    std::string requestFull = buf;
    request->parseHeader(requestFull);
    string msg = "";
    msg = to_string(request->getUid()) + ": \"" + request->returnFirstLine() + "\"from " + ip_address + " @ " + request_time;
    log->save(msg);
    int server_fd = runClient(request->getHost(), request->getPort());
    if (server_fd<0){
      std::string ERROR_404("HTTP/1.1 404 Not Found\r\n\r\n");
      if (send(request->getSocket(), ERROR_404.c_str(), ERROR_404.length(), 0) == -1) {
        throw myException("send 404 Error failed");
      }
      return;
    }
    if (request->getMethod() == "GET"){
      handleGET(request,requestFull, server_fd);
    }
    else if (request->getMethod() == "POST"){
      handlePOST(request,requestFull, server_fd);
    }
    else {// CONNECT
      handleCONNECT(request, server_fd);
    }
  }
  catch(myException e){
    e.what();
  }
  delete request;
}

void proxy::handleGET(Request * request, std::string requestFull, int server_fd){
    Response response;
    if (cache->validate(*request, response)) {
      response = cache->getCache(request->getUrl());
    }
    else {
        if (send(server_fd, requestFull.c_str(), requestFull.size() + 1, 0) == -1){
          throw myException("Error send.");
        }
        std::string received_data = receiveData(BUFFER_SIZE, server_fd);
        response.parseHeader(received_data);
        cache->handle(*request, response);
        close(server_fd);
    }
    sendData(response.getContents(), request->getSocket());
    close(request->getSocket());
}

// need major update #####
void proxy::handlePOST(Request * request, std::string requestFull, int server_fd){
  std::cout<<"###################\n";
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

void proxy::handleCONNECT(Request * request, int server_fd){
  std::cout<<"###################\n";
  //**** 200 OK reponse to client
  // need changes #####
  std::string OK_200("HTTP/1.1 200 Connection Established\r\n\r\n");
  std::cout<<"Notify success CONNECT with server.\n";
  if (send(request->getSocket(), OK_200.c_str(), OK_200.length(), 0) == -1) {
    throw myException("send 200 OK back failed");
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
      //std::cout<<"Receiving data from client.\n";
      len = recv(request->getSocket(), &buf, BUFFER_SIZE, 0);
      if (len < 0) {
        std::cout<<"Failed to recv from client in tunnel:\n";
        break;
      } 
      else if (len == 0) {
        break;
      }
      //std::cout<<"Sending data to server.\n";
      if (send(server_fd, buf, len, 0) < 0){
        std::cout<<"Failed to send to server in tunnel:\n";
        break;
      }
    }
    else if (FD_ISSET(server_fd, &readfds)){
      //std::cout<<"Receiving data from server.\n";
      len = recv(server_fd, &buf, BUFFER_SIZE, 0);
      if (len < 0) {
        std::cout<<"Failed to recv from server in tunnel:\n";
        break;
      } 
      else if (len == 0) {
        break;
      }
      //std::cout<<"Sending data to client.\n";
      if (send(request->getSocket(), buf, len, 0) < 0) {
        std::cout<<"Failed to send to client in tunnel:\n";
        break;
      }
    }
    memset(&buf, 0, sizeof(buf));
  }
  std::cout<<"Tunnel closed.\n";
  close(request->getSocket());
  close(server_fd);
}

// Not done
int proxy::runClient(std::string host, std::string port){
  socketInfo serverSocket = socketInfo(host.c_str(), port.c_str());
  try{
    serverSocket.clientSetup();
  }
  catch(myException e){
    std::cout<<e.what();
    return -1;
  }
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
