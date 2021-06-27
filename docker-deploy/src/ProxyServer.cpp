#include "ProxyServer.h"

ProxyServer::ProxyServer(size_t thread_pool_size, size_t epoller_max_event, int trigger_mode, int port):
                        is_close(false), listen_port(port)
{
    epoll_obj = Epoller(epoller_max_event);
    threadpool_obj = ThreadPool(thread_pool_size);
    if (!InitSocket()){
        is_close = true;    
    }
    InitEventMode(trigger_mode);
}

ProxyServer::~ProxyServer(){
    close(listen_fd);
    is_close = true;
}

void ProxyServer::RunServer(){
    if(!isClose_) { //LOG_INFO("========== Server start =========="); }
    while(!isClose_) {
        // if(timeoutMS_ > 0) {
        //     timeMS = timer_->GetNextTick();
        // }
        int eventCnt = epoll_obj.Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            /* handle event */
            int fd = epoll_obj->GetEventFd(i);
            uint32_t event = epoll_obj->GetEventStatus(i);
            if (event & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                /* when there is error or hang up */
                fprintf (stderr, "epoll error\n");
                close (fd);
                continue;
            }
            if (event & EPOLLIN){
                if (fd == listen_fd){
                    /* handle new connection */
                    this.HandleListen();
                }
                else {
                    /* handle read from socket */
                    this.HandleRead(fd, event);
                    //assert(users_.count(fd) > 0);
                    //this.HandleRead(&users_[fd]);
                }
            }
            else if ((event & EPOLLOUT) && fd != listen_fd){
                /* handle write to socket */
                this.HandleWrite(fd, event);
                //assert(users_.count(fd) > 0);
                //this.HandleWrite(&users_[fd]);
            }
            } else {
                fprintf (stderr, "unexpected error\n");
                //LOG_ERROR("Unexpected event");
            }
            // else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
            //     assert(users_.count(fd) > 0);
            //     CloseConnection(&users_[fd]);
            // }
                
            
        }
    }
}

bool ProxyServer::InitSocket(){
    int ret;
    struct sockaddr_in addr;
    if(listen_port > 65535 || listen_port < 1024) {
        //LOG_ERROR("Port:%d error!",  listen_port);
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
        //LOG_ERROR("Create socket error!", listen_port);
        return false;
    }

    ret = setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listen_fd);
        //LOG_ERROR("Init linger error!", listen_port);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        //LOG_ERROR("set socket setsockopt error !");
        close(listen_fd);
        return false;
    }

    ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        //LOG_ERROR("Bind Port:%d error!", listen_port);
        close(listen_fd);
        return false;
    }

    ret = listen(listen_fd, 6);
    if(ret < 0) {
        //LOG_ERROR("Listen port:%d error!", listen_port);
        close(listen_fd)
        return false;
    }
    ret = epoll_obj.AddFd(listen_fd, EPOLLIN | listen_event)
    if(ret == 0) {
        //LOG_ERROR("Add listen error!");
        close(listen_fd);
        return false;
    }
    this.SetFdNonblock(listen_fd);
    return true;
}

void ProxyServer::InitEventMode(int trigger_mode){
    listen_event = EPOLLRDHUP;
    connection_event = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    //HttpConn::isET = (connEvent_ & EPOLLET);
}

void ProxyServer::HandleListen(){
    do {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        char hbuf[INET_ADDRSTRLEN];
        
        int fd = accept(listen_fd, (struct sockaddr *)&addr, &len);
        if(fd <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                // do nothing
            }
            else {
                perror("accept");
            }
            break;
        }
        inet_ntop(AF_INET, &addr.sin_addr, hbuf, sizeof(hbuf));
        printf("Accepted connection on descriptor %d (host=%s, port=%d)\n", fd, hbuf, addr.sin_port);
        this.SetFdNonBlock(fd);
        if (!epoll_obj.AddFd(fd, EPOLLIN | connection_event)){
            fprintf (stderr, "add fd error\n");
        }
    }while(listen_event & EPOLLET);
}

void ProxyServer::HandleRead(int fd, uint32_t& event){
    while (1) {
        ssize_t count;
        char buf[512];
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
        /* 输出到stdout */
        s = write (1, buf, count);
        if (s == -1) {
            perror ("write");
            abort ();
        }
        event = EPOLLOUT | EPOLLET;
        epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    }
}

void HandleWrite(int fd, uint32_t& event){
    write(fd, "Welcome to nowhere.\n", 20);
    event = EPOLLET | EPOLLIN;
    epoll_ctl(efd, EPOLL_CTL_MOD, fd, &event);
}


int ProxyServer::SetFdNonBlock(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}