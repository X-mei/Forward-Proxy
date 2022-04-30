#include "ProxyServer.h"

ProxyServer::ProxyServer(size_t thread_pool_size, int trigger_mode, int port, bool enable_log, 
                        int log_level, int log_queue_size):is_close(false), listen_port(port), 
                        request_cnt(0), epoll_obj(new Epoller()), threadpool_obj(new ThreadPool(thread_pool_size)),
                        cache_obj(new Cache())
{

    // epoll_obj = new Epoller();
    // threadpool_obj = new ThreadPool(thread_pool_size);
    InitEventMode(trigger_mode);
    if (!InitSocket()){
        is_close = true;    
    }
    if (enable_log){
        Log::Instance()->init(log_level, "./log", ".log", log_queue_size, 1024);
        if (is_close){
            LOG_ERROR("========== Server init error!==========");
        }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d", listen_port);
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listen_event & EPOLLET ? "ET": "LT"),
                            (connection_event & EPOLLET ? "ET": "LT"));
            LOG_INFO("Log level: %d", log_level);
            LOG_INFO("ThreadPool num: %d", thread_pool_size);
        }
    }
}

ProxyServer::~ProxyServer(){
    close(listen_fd);
    is_close = true;
}

void ProxyServer::RunServer(){
    if(!is_close) 
    { 
        LOG_INFO("========== Server started ==========");
    }
    while(!is_close) {
        // if(timeoutMS_ > 0) {
        //     timeMS = timer_->GetNextTick();
        // }
        int eventCnt = epoll_obj->Wait();
        for(int i = 0; i < eventCnt; i++) {
            /* handle event */
            int fd = epoll_obj->GetEventFd(i);
            uint32_t event = epoll_obj->GetEventStatus(i);
            if (fd == listen_fd){
                this->HandleListen();
            }
            else if (event & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)){
                this->CloseConnection(fd, event);
            }
            else if (event & EPOLLIN){
                this->HandleRead(fd, event);
            }
            else if (event & EPOLLOUT){
                this->HandleWrite(fd, event);
            }
            else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

bool ProxyServer::InitSocket(){
    int ret;
    struct sockaddr_in addr;
    // Port out of range
    if(listen_port > 65535 || listen_port < 1024) {
        LOG_ERROR("Port:%d error!",  listen_port);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(listen_port);
    struct linger optLinger = { 0 };
    if(open_linger) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0) {
        LOG_ERROR("Create socket error!", listen_port);
        return false;
    }

    ret = setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listen_fd);
        LOG_ERROR("Init linger error!", listen_port);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listen_fd);
        return false;
    }

    ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", listen_port);
        close(listen_fd);
        return false;
    }

    ret = listen(listen_fd, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", listen_port);
        close(listen_fd);
        return false;
    }

    ret = epoll_obj->AddFd(listen_fd, EPOLLIN | listen_event);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listen_fd);
        return false;
    }

    // Set the epoll fd non blocking
    this->SetFdNonBlock(listen_fd);
    LOG_INFO("Server init complete, listenning on port: %d", listen_port);
    return true;
}

void ProxyServer::InitEventMode(int trigger_mode){
    listen_event = EPOLLRDHUP;
    connection_event = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigger_mode)
    {
    case 0:
        break;
    case 1:
        connection_event |= EPOLLET;
        break;
    case 2:
        listen_event |= EPOLLET;
        break;
    case 3:
        listen_event |= EPOLLET;
        connection_event |= EPOLLET;
        break;
    default:
        listen_event |= EPOLLET;
        connection_event |= EPOLLET;
        break;
    }
    // HttpConn::isET = (connEvent_ & EPOLLET);
}

void ProxyServer::CloseConnection(int fd, uint32_t& event){
    if (fd_to_id[fd] == 0){
        // Need further handling
    }
    else {
        LOG_INFO("%d: Client[%d] quit!", fd_to_id[fd], fd);
    }
    epoll_obj->DelFd(fd);
    close(fd);
}

void ProxyServer::HandleListen(){
    do {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        char hbuf[INET_ADDRSTRLEN];
        int fd = accept(listen_fd, (struct sockaddr *)&addr, &len);
        if(fd <= 0) {
            return;
            // if ((errno == EAGAIN) || (errno == EWOULDBLOCK)){
            //     // do nothing
            // }
            // else {
            //     perror("accept");
            // }
            // break;
        }
        inet_ntop(AF_INET, &addr.sin_addr, hbuf, sizeof(hbuf));
        request_cnt++;
        fd_to_id[fd] = request_cnt;
        LOG_INFO("%d: Accepted connection on descriptor %d (host=%s, port=%d)", request_cnt, fd, hbuf, addr.sin_port);
        if (!epoll_obj->AddFd(fd, EPOLLIN | connection_event)){
            LOG_ERROR("%d: Add fd error");
        }
        this->SetFdNonBlock(fd);
    }while(listen_event & EPOLLET);
}

void ProxyServer::HandleRead(int fd, uint32_t& event){
    vector<char> headers;
    try{
        ReceiveData(fd, headers);
    }
    catch(const std::exception& ex){
        LOG_ERROR("%d: %s", fd_to_id[fd], ex.what());
        close(fd);
        return;
    }
    threadpool_obj->enqueue(bind(&ProxyServer::ProcessRequest, this, headers, fd, request_cnt));
}

void ProxyServer::HandleWrite(int fd, uint32_t& event){
    // const std::string OK_200("HTTP/1.1 200 Connection Established\r\n\r\nWelcome to nowhere.");
    // size_t n = OK_200.size();
    // write(fd, OK_200, n);
    try{
        LOG_INFO("%d: Responding %s", fd_to_id[fd], pending_response[fd].getFirstLine().c_str());
        SendData(pending_response[fd].getCompleteMessage(), fd);
        pending_response.erase(fd);
    }
    catch(const std::exception& ex){
        LOG_ERROR("%d: %s", fd_to_id[fd], ex.what());
    }
    close(fd);
    // epoll_obj->ModFd(fd, connection_event | EPOLLIN);
}

void ProxyServer::ProcessRequest(vector<char>& requestFull, int client_fd, int request_id){
    Request* request = new Request(client_fd, request_id);
    try{
        request->parseHeader(requestFull);
    }
    catch(const std::exception& ex){
        LOG_ERROR("%d: %s", request->getUid(), ex.what());
        return;
    }
    LOG_INFO("%d: %s", request->getUid(), request->getFirstLine().c_str());
    int server_fd = RunClient(request->getHost(), request->getPort(), client_fd);
    try{
        if (server_fd<0){
            std::string ERROR_404("HTTP/1.1 404 Not Found\r\n\r\n");
            if (send(request->getSocket(), ERROR_404.c_str(), ERROR_404.length(), 0) == -1) {
                throw myException("Send 404 Error failed");
            }
            close(server_fd);
            close(request->getSocket());
            return;
        }
        if (request->getMethod() == "GET"){
            HandleGET(request, server_fd);
        }
        else if (request->getMethod() == "POST"){
            HandlePOST(request, server_fd);
        }
        else {
            HandleCONNECT(request, server_fd);
        }
    }
    catch(const std::exception& ex){
        LOG_ERROR("%d: %s", fd_to_id[client_fd], ex.what());
    }
    catch(...){
        LOG_ERROR("%d: %s", fd_to_id[client_fd], "Unexpected error occured.");
    }
    close(server_fd);
    delete request;
}

int ProxyServer::RunClient(std::string host, std::string port, int client_fd){
    socketInfo serverSocket = socketInfo(host.c_str(), port.c_str());
    try{
        serverSocket.clientSetup();
        serverSocket.socketConnect();
    }
    catch(const std::exception& ex){
        LOG_ERROR("%d: %s", fd_to_id[client_fd], ex.what());
        return -1;
    }
    return serverSocket.getFd();
}

void ProxyServer::HandleGET(Request* request, int server_fd){
    Response response;
    int client_fd = request->getSocket();
    if (cache_obj->validate(*request, response)) {
        response = cache_obj->getCache(request->getUrl());
        pending_response[client_fd] = response;
        epoll_obj->ModFd(client_fd, EPOLLOUT | EPOLLET);
        // SendData(response.getCompleteMessage(), request->getSocket());
    }
    else {
        LOG_INFO("%d: Requesting %s", request->getUid(), request->getFirstLine().c_str());
        SendData(request->getCompleteMessage(), server_fd);
        vector<char> headers(BUFFER_SIZE);
        ReceiveOneChunk(server_fd, headers);
        response.parseHeader(headers);
        LOG_INFO("%d: Received %s from destination server", request->getUid(), response.getFirstLine().c_str());
        // if the response is chunked, send back the initial chunk, then send back every chunk until chunk size is zero
        if (response.checkIfChunked()){
            LOG_INFO("%d: Responding chunked message", request->getUid());
            SendData(response.getCompleteMessage(), client_fd);
            vector<char> target{'0','\r','\n','\r','\n'};
            vector<char> temp(BUFFER_SIZE);
            while (true){
                vector<char> temp(BUFFER_SIZE);
                ReceiveOneChunk(server_fd, temp);
                SendData(temp, client_fd);
                auto it = std::search(temp.begin(), temp.end(), target.begin(), target.end());
                if (it != temp.end()){
                    break;
                }
            }
        }
        else {
            // if not chunked, acquire all the missing bodies and send back the response
            while (response.getBodySizeLeft()>0){
                vector<char> temp(BUFFER_SIZE);
                ReceiveOneChunk(server_fd, temp);
                response.addMissingBody(temp);
            }
            pending_response[client_fd] = response;
            epoll_obj->ModFd(client_fd, EPOLLOUT | EPOLLET);
        }
    }
}

void ProxyServer::HandlePOST(Request* request, int server_fd){
    LOG_INFO("%d: Requesting %s", request->getUid(), request->getFirstLine().c_str());
    SendData(request->getCompleteMessage(), server_fd);
    Response response;
    int client_fd = request->getSocket();
    vector<char> headers(BUFFER_SIZE);
    ReceiveOneChunk(server_fd, headers);
    response.parseHeader(headers);
    LOG_INFO("%d: Received %s from destination server", request->getUid(), response.getFirstLine().c_str());
    while (response.getBodySizeLeft()>0){
        vector<char> temp(BUFFER_SIZE);
        ReceiveOneChunk(server_fd, temp);
        response.addMissingBody(temp);
    }
    pending_response[client_fd] = response;
    epoll_obj->ModFd(client_fd, EPOLLOUT | EPOLLET);
}

void ProxyServer::HandleCONNECT(Request* request, int server_fd){
    int client_fd = request->getSocket();
    // for some reason have to set fd to blocking, haven't found a circumvent yet
    // this->SetFdBlock(client_fd);
    LOG_INFO("%d: Responding %s", request->getUid(), "HTTP/1.1 200 OK");
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    fd_set readable;
    int nfds;
    if(client_fd < server_fd) {
        nfds = 1 + server_fd;
    } else {
        nfds = 1 + client_fd;
    }

    //continuously listen to the client request
    while (1) {
        FD_ZERO(&readable);
        FD_SET(server_fd, &readable);
        FD_SET(client_fd, &readable);

        select(nfds, &readable, NULL, NULL, NULL);

        char message[65536];
        //step1: receive request message from client
        memset(message, 0, sizeof(message));
        if(FD_ISSET(server_fd, &readable)) {
            int len_recv = recv(server_fd, message, sizeof(message), 0);
            if(len_recv <= 0) {
                break;
            }
            // else if (len_recv == -1 && errno != EAGAIN){
            //     break;
            // }
            else {
                //if the request message is valid, then send the message to server
                int len_send = send(client_fd, message, len_recv, 0);
                if(len_send <= 0) {
                    break;
                }
            }
        }

        //step 2: receive response message from server
        memset(message, 0, sizeof(message));
        if(FD_ISSET(client_fd, &readable)) {
            int len_recv = recv(client_fd, message, sizeof(message), 0);
            if(len_recv <= 0) {
                break;
            }
            // else if (len_recv == -1 && errno != EAGAIN){
            //     break;
            // }
            else {
                //if the response message is valid, then return the message to client
                int len_send = send(server_fd, message, len_recv, 0);
                if(len_send <= 0) {
                    return;
                }
            }
        }
    }
    // set it back to non-blocking
    // this->SetFdNonBlock(client_fd);
    LOG_INFO("%d: Tunnel closed", request->getUid());
}

// Receive the maximum possible amount of data
void ProxyServer::ReceiveData(int source_fd, vector<char>& data){
    int start = 0;
    int byte_count = 0;
    while (1) {
        vector<char> temp(BUFFER_SIZE);
        byte_count = recv(source_fd, &temp.data()[0], BUFFER_SIZE, 0);
        if (byte_count == 0) {
            break;
        }
        if (byte_count == -1) {
            if (errno != EAGAIN){
                throw myException("Error recv");
            }
            else {
                break;
            }
        }
        temp.resize(byte_count);
        data.insert(data.end(), temp.begin(), temp.end());
    }
}

// Receive the first chunk, which should contain the header
void ProxyServer::ReceiveOneChunk(int source_fd, vector<char>& headers){
    ssize_t recv_size = recv(source_fd, &headers.data()[0], BUFFER_SIZE, 0);
    if (recv_size < 0 && errno != EAGAIN) {
        throw myException("Error recv");
    }
    headers.resize(recv_size);
    return;
}

void ProxyServer::SendData(vector<char> data, int dest_fd){
    int bytes_total = data.size();
    int bytes_left = bytes_total;
    int bytes_sent = 0;
    int bytes_count;
    while (bytes_sent < bytes_total) {
        if ((bytes_count = send(dest_fd, &data.data()[bytes_sent], bytes_left, 0)) == -1) {
            throw myException("Error send");
        }
        bytes_sent += bytes_count;
        bytes_left -= bytes_count;
    }
}

int ProxyServer::SetFdNonBlock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

int ProxyServer::SetFdBlock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) & ~O_NONBLOCK);
}
