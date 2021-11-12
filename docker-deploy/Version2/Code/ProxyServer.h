#ifndef PROXYSERVER_H
#define PROXYSERVER_H
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include "Log.h"
#include "Epoll.h"
#include "ThreadPool.h"

class ProxyServer{
private:
    int listen_fd;
    int is_close;
    int listen_port;
    bool open_linger;
    // RAII management of self-defined object
    std::unique_ptr<Epoller> epoll_obj;
    std::unique_ptr<ThreadPool> threadpool_obj;
    // Epoller* epoll_obj;
    // ThreadPool* threadpool_obj;

    uint32_t listen_event;
    uint32_t connection_event;

    static int SetFdNonBlock(int fd);
    
    bool InitSocket();

    void InitEventMode(int trigger_mode);

    void HandleListen();

    void HandleRead(int fd, uint32_t& event);

    void HandleWrite(int fd, uint32_t& event);

    void CloseConnection(int fd, uint32_t& event);

public:
    ProxyServer(size_t thread_pool_size, int trigger_mode, int port, bool enable_log,
                int log_level, int log_queue_size);

    ~ProxyServer();

    void RunServer();
    
};

#endif