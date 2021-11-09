#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

class Epoller {
private:
    // fd of the epoller core object
    int epollfd;
    // all the epoll event obejct added
    std::vector<struct epoll_event> events;

public:
    explicit Epoller(size_t maxEvent = 1024);

    ~Epoller();

    bool AddFd(int fd, uint32_t event_status);

    bool ModFd(int fd, uint32_t event_status);

    bool DelFd(int fd);

    int Wait(int timeoutMs = -1);

    int GetEventFd(size_t ind) const;

    uint32_t GetEventStatus(size_t ind) const;
};

#endif