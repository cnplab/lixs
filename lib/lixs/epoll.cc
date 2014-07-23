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

void lixs::epoll::once(iok& k)
{
    once_lst.push_front(&k);
}

void lixs::epoll::add(iokfd& k, int fd, const iokfd::ioev& ev)
{
    struct epoll_event event = { get_events(ev), { reinterpret_cast<void*>(&k) } };

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

void lixs::epoll::set(iokfd& k, int fd, const iokfd::ioev& ev)
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

    /* Run once events */
    for(std::list<iok*>::iterator i = once_lst.begin(); i != once_lst.end(); i++) {
        (*i)->run();
    }
    once_lst.clear();

    /* Run IO events */
    n_events = epoll_wait(epfd, epev, epoll_max_events, timeout);
    for (int i = 0; i < n_events; i++) {
        iokfd::ioev ev = get_events(epev[i].events);
        iokfd* k = reinterpret_cast<iokfd*>(epev[i].data.ptr);

        k->handle(ev);
    }

}

uint32_t inline lixs::epoll::get_events(const struct iokfd::ioev& ev)
{
    return (ev.read ? EPOLLIN : 0) | (ev.write ? EPOLLOUT : 0);
}

lixs::iokfd::ioev inline lixs::epoll::get_events(uint32_t ev)
{
    return iokfd::ioev(ev && EPOLLIN != 0, ev && EPOLLOUT != 0);
}

