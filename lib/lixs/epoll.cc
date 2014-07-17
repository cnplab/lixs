#include <lixs/epoll.hh>

#include <cstddef>
#include <sys/epoll.h>


lixs::epoll::epoll(void)
{
    epfd = epoll_create(0x7E57);
}

lixs::epoll::~epoll()
{
}

int lixs::epoll::add(int fd, const struct events& ev, struct ptr* ptr)
{
    struct epoll_event event = { get_events(ev), { reinterpret_cast<void*>(ptr) } };

    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

int lixs::epoll::set(int fd, const struct events& ev, struct ptr* ptr)
{
    struct epoll_event event = { get_events(ev), { reinterpret_cast<void*>(ptr) } };

    return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}

int lixs::epoll::remove(int fd)
{
    /* Passing event == NULL requires linux > 2.6.9, see BUGS */
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

int lixs::epoll::handle(void)
{
    int n_events;
    struct ptr* ptr;

    n_events = epoll_wait(epfd, events, epoll_max_events, timeout);

    for (int i = 0; i < n_events; i++) {
        ptr = reinterpret_cast<struct ptr*>(events[i].data.ptr);
        ptr->fn(ptr);
    }

    return 0;
}

uint32_t inline lixs::epoll::get_events(const struct events& ev)
{
    return (ev.read ? EPOLLIN : 0) | (ev.write ? EPOLLOUT : 0);
}

