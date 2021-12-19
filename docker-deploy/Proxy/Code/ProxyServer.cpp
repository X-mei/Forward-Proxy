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
        Log::Instance()->init(log_level, "./log", ".log", log_queue_size);
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
    // delete epoll_obj;
    // ddelete threadpool_obj;
}

void ProxyServer::RunServer(){
    if(!is_close) 
    { 
        LOG_INFO("========== Server start ==========");
    }
    while(!is_close) {
        // if(timeoutMS_ > 0) {
        //     timeMS = timer_->GetNextTick();
        // }
        int eventCnt = epoll_obj->Wait();
        //std::cout << "Should only appear once." << std::endl;
        for(int i = 0; i < eventCnt; i++) {
            /* handle event */
            int fd = epoll_obj->GetEventFd(i);
            uint32_t event = epoll_obj->GetEventStatus(i);
            if (fd == listen_fd){
                std::cout << "Handling new connection." << std::endl;
                this->HandleListen();
            }
            else if (event & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)){
                std::cout << "Communication done." << std::endl;
                this->CloseConnection(fd, event);
            }
            else if (event & EPOLLIN){
                std::cout << "Handling read from socket." << std::endl;
                this->HandleRead(fd, event);
            }
            else if (event & EPOLLOUT){
                std::cout << "Handling write to socket." << std::endl;
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
    std::cout << "Server init complete, start listening for request." << std::endl;
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
    std::cout << "Event mode setting done." << std::endl;
    //HttpConn::isET = (connEvent_ & EPOLLET);
}

void ProxyServer::CloseConnection(int fd, uint32_t& event){
    LOG_INFO("Client[%d] quit!", fd);
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
        printf("Accepted connection on descriptor %d (host=%s, port=%d)\n", fd, hbuf, addr.sin_port);
        request_cnt++;
        if (!epoll_obj->AddFd(fd, EPOLLIN | connection_event)){
            fprintf (stderr, "add fd error\n");
        }
        this->SetFdNonBlock(fd);
    }while(listen_event & EPOLLET);
}

void ProxyServer::HandleRead(int fd, uint32_t& event){
    std::stringstream ss;
    while (1) {
        ssize_t count;
        char buf[4096];
        count = read(fd, buf, sizeof(buf));
        if (count == -1) {
            if (errno != EAGAIN) {
                perror ("read");
                close(fd);
            }
            break;
        } else if (count == 0) {
            /* 数据读取完毕，结束 */
            close(fd);
            printf ("Closed connection on descriptor %d\n", fd);
            break;
        }
        ss << buf;
        /* 输出到stdout */
        // if (write(1, buf, count) == -1) {
        //     perror ("write");
        //     abort ();
        // }
    }
    ss << '\0';
    // std::string requestFull = buf;
    threadpool_obj->enqueue(bind(&ProxyServer::ProcessRequest, this, ss.str(), fd, request_cnt));
}

void ProxyServer::HandleWrite(int fd, uint32_t& event){
    // const std::string OK_200("HTTP/1.1 200 Connection Established\r\n\r\nWelcome to nowhere.");
    // size_t n = OK_200.size();
    // write(fd, OK_200, n);
    try{
        SendData(pending_response[fd].getCompleteMessage(), fd);
        pending_response.erase(fd);
        
    }
    catch(myException e){
        std::cout<<e.what();
    }
    epoll_obj->ModFd(fd, connection_event | EPOLLIN);
    // write(fd, "HTTP/1.1 200 Connection Established\r\n\r\nWelcome to nowhere.", 58);
}

void ProxyServer::ProcessRequest(std::string requestFull, int client_fd, int request_id){
    Request * request = new Request(client_fd, request_id);
    request->parseHeader(requestFull);
    int server_fd = RunClient(request->getHost(), request->getPort());
    try{
        if (server_fd<0){
            std::string ERROR_404("HTTP/1.1 404 Not Found\r\n\r\n");
            if (send(request->getSocket(), ERROR_404.c_str(), ERROR_404.length(), 0) == -1) {
                throw myException("send 404 Error failed");
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
        else {// CONNECT
            HandleCONNECT(request, server_fd);
        }
    }
    catch(myException e){
        std::cout<<e.what()<<std::endl;
    }
    catch(...){
        //do nothing
    }
    epoll_obj->ModFd(client_fd, EPOLLOUT | EPOLLET);
    // close(request->getSocket());
    close(server_fd);
    delete request;
}

int ProxyServer::RunClient(std::string host, std::string port){
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

void ProxyServer::HandleGET(Request * request, int server_fd){
    Response response;
    if (cache_obj->validate(*request, response)) {
        response = cache_obj->getCache(request->getUrl());
    }
    else {
        if (send(server_fd, request->getCompleteMessage().c_str(), request->getCompleteMessage().size() + 1, 0) == -1){
            throw myException("Error send.");
        }
        // std::string msg = to_string(request->getUid()) + ": Requesting \"" + request->returnFirstLine() + "\" from " + request->getHost();
        // log->save(msg);
        std::string received_data = ReceiveData(BUFFER_SIZE, server_fd);
        response.parseHeader(received_data);
        // msg = to_string(request->getUid()) + ": Recieved \"" + response.returnFirstLine() + "\" from " + request->getHost();
        // log->save(msg);
        cache_obj->handle(*request, response);
    }
    pending_response[request->getSocket()] = response;
    // sendData(response.getContents(), request->getSocket());
    // std::string msg = to_string(request->getUid()) + ": Responding \"" + response.returnFirstLine();
    // log->save(msg);
}

// need major update
void ProxyServer::HandlePOST(Request * request, int server_fd){
    std::cout<<"###################\n";
    if (send(server_fd, request->getCompleteMessage().c_str(), request->getCompleteMessage().size() + 1, 0) == -1){
        throw myException("Error send.");
    }
    // std::string msg = to_string(request->getUid()) + ": Requesting \"" + request->returnFirstLine() + "\" from " + request->getHost();
    // log->save(msg);
    std::string received_data = ReceiveData(BUFFER_SIZE, server_fd);
    Response response(server_fd, 0);
    response.parseHeader(received_data);
    // msg = to_string(request->getUid()) + ": Recieved \"" + response.returnFirstLine() + "\" from " + request->getHost();
    // log->save(msg);
    pending_response[request->getSocket()] = response;
    // msg = to_string(request->getUid()) + ": Responding \"" + response.returnFirstLine();
    // log->save(msg);
    
    // sendData(received_data, request->getSocket());
}

void ProxyServer::HandleCONNECT(Request * request, int server_fd){
    std::cout<<"###################\n";
    std::string OK_200("HTTP/1.1 200 Connection Established\r\n\r\n");
    std::cout<<"Notify success CONNECT with server.\n";
    if (send(request->getSocket(), OK_200.c_str(), OK_200.length(), 0) == -1) {
        throw myException("send 200 OK back failed");
    }
    // std::string msg = to_string(request->getUid()) + ": " + "Opening Tunnel";
    // log->save(msg);
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
                std::cout<<"Failed to recv from client in tunnel:\n";
                break;
            } 
            else if (len == 0) {
                break;
            }
            if (send(server_fd, buf, len, 0) < 0){
                std::cout<<"Failed to send to server in tunnel:\n";
                break;
            }
            }
            else if (FD_ISSET(server_fd, &readfds)){
            len = recv(server_fd, &buf, BUFFER_SIZE, 0);
            if (len < 0) {
                std::cout<<"Failed to recv from server in tunnel:\n";
                break;
            } 
            else if (len == 0) {
                break;
            }
            if (send(request->getSocket(), buf, len, 0) < 0) {
                std::cout<<"Failed to send to client in tunnel:\n";
                break;
            }
        }
        memset(&buf, 0, sizeof(buf));
    }
    // msg = to_string(request->getUid()) + ": " + "Tunnel closed";
    // log->save(msg);
}

std::string ProxyServer::ReceiveData(int buf_size, int source_fd){
    std::string data = "";
    while (1) {
        char buf[buf_size];
        int byte_count;
        if ((byte_count = recv(source_fd, buf, buf_size, 0)) == -1) {
            throw myException("Error recv");
        }
        if (byte_count == 0) {
            break;
        }  
        // Append part of received string 
        data.append(buf);
    }
    return data;
}

void ProxyServer::SendData(std::string data, int dest_fd){
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

int ProxyServer::SetFdNonBlock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}