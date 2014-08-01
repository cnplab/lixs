#ifndef __LIXS_EPOLL_HH__
#define __LIXS_EPOLL_HH__

#include <lixs/iomux.hh>

#include <list>
#include <sys/epoll.h>


namespace lixs {

class epoll : public iomux {
public:
    epoll(void);
    ~epoll();

    void once(ev_cb& k);

    void add(fd_cb& k, int fd, const fd_cb::fd_ev& ev);
    void set(fd_cb& k, int fd, const fd_cb::fd_ev& ev);
    void remove(int fd);

    void handle(void);


private:
    uint32_t inline get_events(const fd_cb::fd_ev& ev);
    fd_cb::fd_ev inline get_events(uint32_t ev);


private:
    /* TODO: make this configurable */
    static const int timeout = 100;
    static const int epoll_max_events = 1000;

    int epfd;
    struct epoll_event epev[epoll_max_events];

    std::list<ev_cb*> once_lst;
};

} /* namespace lixs */

#endif /* __LIXS_EPOLL_HH__ */

