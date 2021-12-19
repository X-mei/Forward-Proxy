#include "Epoll.h"

Epoller::Epoller(size_t maxEvent): events(maxEvent){
    epollfd = epoll_create(512);
    assert(epollfd >= 0 && events.size()>0);
}

Epoller::~Epoller(){
    close(epollfd);
}

bool Epoller::AddFd(int fd, uint32_t event_status){
    if (fd < 0) return false;
    struct epoll_event event;
    event.data.fd = fd;
    event.events = event_status;
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

bool Epoller::ModFd(int fd, uint32_t event_status){
    if (fd < 0) return false;
    struct epoll_event event;
    event.data.fd = fd;
    event.events = event_status;
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

bool Epoller::DelFd(int fd){
    if (fd < 0) return false;
    struct epoll_event event;
    return 0 == epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event);
}

int Epoller::Wait(int timeoutMs){
    return epoll_wait(epollfd, &events[0], static_cast<int>(events.size()), timeoutMs);
}

int Epoller::GetEventFd(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].data.fd;
}

uint32_t Epoller::GetEventStatus(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].events;
}