#ifndef PROXYSERVER_H
#define PROXYSERVER_H
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Epoller.h"
#include "ThreadPool.h"

class ProxyServer{
private:
    int listen_fd;
    int is_close;
    int listen_port;
    bool open_linger;
    Epoller epoll_obj;
    ThreadPool threadpool_obj;

    uint32_t listen_event;
    uint32_t connection_event

    static int SetFdNonBlock(int fd);
    
    bool InitSocket();

    void InitEventMode(int trigger_mode);

    void HandleListen();

    void HandleRead();

    void HandleWrite();

    void CloseConnection();

    void 

public:
    ProxyServer(size_t thread_pool_size, size_t epoller_max_event, int trigger_mode);

    ~ProxyServer();

    void RunServer();

    
};

#endif