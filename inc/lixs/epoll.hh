#ifndef __LIXS_EPOLL_HH__
#define __LIXS_EPOLL_HH__

#include <lixs/iomux.hh>

#include <sys/epoll.h>


namespace lixs {

class epoll : public iomux {
public:
    epoll(void);
    ~epoll();

    void add(fd_cb_k& cb);
    void set(fd_cb_k& cb);
    void remove(fd_cb_k& cb);

    void handle(void);


private:
    uint32_t inline get_events(const fd_cb_k& cb);
    bool inline is_read(const uint32_t ev);
    bool inline is_write(const uint32_t ev);


private:
    /* TODO: make this configurable */
    static const int timeout = 100;
    static const int epoll_max_events = 1000;

    int epfd;
    struct epoll_event epev[epoll_max_events];
};

} /* namespace lixs */

#endif /* __LIXS_EPOLL_HH__ */

