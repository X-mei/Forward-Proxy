#include "Proxy.h"

void proxy::runServer(){
  serverSocket.serverSetup();
  serverSocket.socketWaitConnect();
}