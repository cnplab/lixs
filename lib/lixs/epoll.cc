#include <lixs/epoll.hh>

#include <cstddef>
#include <cstdio>
#include <list>
#include <sys/epoll.h>


lixs::epoll::epoll(void)
    : epfd(epoll_create(0x7E57))
{
}

lixs::epoll::~epoll()
{
}

void lixs::epoll::add(fd_cb& k, int fd, const fd_cb::fd_ev& ev)
{
    struct epoll_event event = { get_events(ev), { reinterpret_cast<void*>(&k) } };

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

void lixs::epoll::set(fd_cb& k, int fd, const fd_cb::fd_ev& ev)
{
    struct epoll_event event = { get_events(ev), { reinterpret_cast<void*>(&k) } };

    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}

void lixs::epoll::remove(int fd)
{
    /* Passing event == NULL requires linux > 2.6.9, see BUGS */
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

void lixs::epoll::handle(void)
{
    int n_events;

    /* Run IO events */
    n_events = epoll_wait(epfd, epev, epoll_max_events, timeout);
    for (int i = 0; i < n_events; i++) {
        fd_cb::fd_ev ev = get_events(epev[i].events);
        fd_cb* k = reinterpret_cast<fd_cb*>(epev[i].data.ptr);

        k->handle(ev);
    }
}

uint32_t inline lixs::epoll::get_events(const struct fd_cb::fd_ev& ev)
{
    return (ev.read ? EPOLLIN : 0) | (ev.write ? EPOLLOUT : 0);
}

lixs::fd_cb::fd_ev inline lixs::epoll::get_events(uint32_t ev)
{
    return fd_cb::fd_ev(ev && EPOLLIN != 0, ev && EPOLLOUT != 0);
}

