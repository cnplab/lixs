#include <lixs/os_linux/epoll.hh>

#include <cstddef>
#include <cstdio>
#include <list>
#include <sys/epoll.h>


lixs::os_linux::epoll::epoll(event_mgr& emgr)
    : iomux(emgr), epfd(epoll_create(0x7E57))
{
    emgr.enqueue_event(std::bind(&epoll::handle, this));
}

lixs::os_linux::epoll::~epoll()
{
}

void lixs::os_linux::epoll::add(int fd, bool read, bool write, io_cb cb)
{
    std::pair<cb_map::iterator, bool> res = callbacks.insert({fd, cb});

    if (!res.second) {
        return;
    }

    struct epoll_event event = {
        get_events(read, write),
        { static_cast<void*>(&(res.first->second)) }
    };

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

void lixs::os_linux::epoll::set(int fd, bool read, bool write)
{
    cb_map::iterator it = callbacks.find(fd);

    if (it == callbacks.end()) {
        return;
    }

    struct epoll_event event = {
        get_events(read, write),
        { static_cast<void*>(&(it->second)) }
    };

    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}

void lixs::os_linux::epoll::rem(int fd)
{
    cb_map::size_type res = callbacks.erase(fd);

    if (res == 0) {
        return;
    }

    /* Passing event == NULL requires linux > 2.6.9, see BUGS */
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

void lixs::os_linux::epoll::handle(void)
{
    io_cb* cb;
    int n_events;

    n_events = epoll_wait(epfd, epev, epoll_max_events, timeout);

    for (int i = 0; i < n_events; i++) {
        if (!is_err(epev[i].events)) {
            /* TODO: A better design would be to enqueue each of the callbacks
             * on the event_mgr. However it's possible the object the callback
             * refers to is deleted after enqueue and before being fired
             * leading to a crash. Therefore we need to come up with a way to
             * invalidate this pointers before doing that move. Probably this
             * can be done through the use of smart pointers.
             */
            cb = static_cast<io_cb*>(epev[i].data.ptr);
            cb->operator()(is_read(epev[i].events), is_write(epev[i].events));
        }
    }

    emgr.enqueue_event(std::bind(&epoll::handle, this));
}

uint32_t inline lixs::os_linux::epoll::get_events(bool read, bool write)
{
    return (read ? EPOLLIN : 0) | (write ? EPOLLOUT : 0);
}

bool inline lixs::os_linux::epoll::is_read(const uint32_t ev)
{
    return (ev & EPOLLIN) != 0;
}

bool inline lixs::os_linux::epoll::is_write(const uint32_t ev)
{
    return (ev & EPOLLOUT) != 0;
}

bool inline lixs::os_linux::epoll::is_err(const uint32_t ev)
{
    return (ev & EPOLLERR) != 0;
}

