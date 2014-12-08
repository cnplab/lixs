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

void lixs::epoll::add(fd_cb_k& cb)
{
    struct epoll_event event = { get_events(cb), { reinterpret_cast<void*>(&cb) } };

    epoll_ctl(epfd, EPOLL_CTL_ADD, cb.fd, &event);
}

void lixs::epoll::set(fd_cb_k& cb)
{
    struct epoll_event event = { get_events(cb), { reinterpret_cast<void*>(&cb) } };

    epoll_ctl(epfd, EPOLL_CTL_MOD, cb.fd, &event);
}

void lixs::epoll::remove(fd_cb_k& cb)
{
    /* Passing event == NULL requires linux > 2.6.9, see BUGS */
    epoll_ctl(epfd, EPOLL_CTL_DEL, cb.fd, NULL);
}

void lixs::epoll::handle(void)
{
    fd_cb_k* cb;
    int n_events;

    n_events = epoll_wait(epfd, epev, epoll_max_events, timeout);

    for (int i = 0; i < n_events; i++) {
        if (!is_err(epev[i].events)) {
            cb = reinterpret_cast<fd_cb_k*>(epev[i].data.ptr);
            cb->operator()(is_read(epev[i].events), is_write(epev[i].events));
        }
    }
}

uint32_t inline lixs::epoll::get_events(const fd_cb_k& cb)
{
    return (cb.ev_read ? EPOLLIN : 0) | (cb.ev_write ? EPOLLOUT : 0);
}

bool inline lixs::epoll::is_read(const uint32_t ev)
{
    return (ev & EPOLLIN) != 0;
}

bool inline lixs::epoll::is_write(const uint32_t ev)
{
    return (ev & EPOLLOUT) != 0;
}

bool inline lixs::epoll::is_err(const uint32_t ev)
{
    return (ev & EPOLLERR) != 0;
}

