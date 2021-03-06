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
#include <unordered_map>
#include <sstream>

#include "Log.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include "socket.h"
#include "Request.h"
#include "Response.h"
#include "Cache.h"

#define BUFFER_SIZE 4096

class ProxyServer{
private:
    int listen_fd;
    int is_close;
    int listen_port;
    bool open_linger;
    // RAII management of self-defined object
    std::unique_ptr<Epoller> epoll_obj;
    std::unique_ptr<ThreadPool> threadpool_obj;
    std::unique_ptr<Cache> cache_obj;
    // Epoller* epoll_obj;
    // ThreadPool* threadpool_obj;

    uint32_t listen_event;
    uint32_t connection_event;
    int request_cnt;
    std::unordered_map<int, Response> pending_response;
    std::unordered_map<int, int> fd_to_id;

    static int SetFdNonBlock(int fd);

    static int SetFdBlock(int fd);
    
    bool InitSocket();

    void InitEventMode(int trigger_mode);

    void HandleListen();

    void HandleRead(int fd, uint32_t& event);

    void HandleWrite(int fd, uint32_t& event);

    void ProcessRequest(vector<char>& requestFull, int fd, int request_cnt);

    int RunClient(std::string host, std::string port, int client_fd);

    void HandleGET(Request* request, int server_fd);

    void HandlePOST(Request* request, int server_fd);

    void HandleCONNECT(Request* request, int server_fd);

    void ReceiveData(int source_fd, vector<char>& data);

    void ReceiveOneChunk(int source_fd, vector<char>& headers);

    void SendData(vector<char> data, int dest_fd);

    void CloseConnection(int fd, uint32_t& event);

public:
    ProxyServer(size_t thread_pool_size, int trigger_mode, int port, bool enable_log,
                int log_level, int log_queue_size);

    ~ProxyServer();

    void RunServer();
    
};

#endif