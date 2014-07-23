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

    void once(iok& k);

    void add(iokfd& k, int fd, const iokfd::ioev& ev);
    void set(iokfd& k, int fd, const iokfd::ioev& ev);
    void remove(int fd);

    void handle(void);


private:
    uint32_t inline get_events(const iokfd::ioev& ev);
    iokfd::ioev inline get_events(uint32_t ev);


private:
    /* TODO: make this configurable */
    static const int timeout = 100;
    static const int epoll_max_events = 1000;

    int epfd;
    struct epoll_event epev[epoll_max_events];

    std::list<iok*> once_lst;
};

} /* namespace lixs */

#endif /* __LIXS_EPOLL_HH__ */

