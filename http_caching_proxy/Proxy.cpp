#include "Proxy.h"

void proxy::runServer(){
  serverSocket.serverSetup();
  serverSocket.socketWaitConnect();
  while(1){
    serverSocket.socketAccept();
  }
}