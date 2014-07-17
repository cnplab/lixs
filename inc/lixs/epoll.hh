#ifndef __LIXS_EPOLL_HH__
#define __LIXS_EPOLL_HH__

#include <lixs/iomux.hh>

#include <sys/epoll.h>


namespace lixs {

class epoll : public iomux {
public:
    epoll(void);
    ~epoll();

    int add(int fd, const struct events& ev, struct ptr* ptr);
    int set(int fd, const struct events& ev, struct ptr* ptr);
    int remove(int fd);

    int handle(void);


private:
    uint32_t inline get_events(const struct events& ev);


private:
    /* TODO: make this configurable */
    static const int timeout = 100;
    static const int epoll_max_events = 1000;

    int epfd;
    struct epoll_event events[epoll_max_events];
};

} /* namespace lixs */

#endif /* __LIXS_EPOLL_HH__ */

