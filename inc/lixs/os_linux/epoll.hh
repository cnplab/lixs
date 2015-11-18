#ifndef __LIXS_OS_LINUX_EPOLL_HH__
#define __LIXS_OS_LINUX_EPOLL_HH__

#include <lixs/iomux.hh>

#include <map>
#include <sys/epoll.h>


namespace lixs {
namespace os_linux {

class epoll : public iomux {
public:
    epoll(event_mgr& emgr);
    ~epoll();

    void add(int fd, bool read, bool write, io_cb cb);
    void set(int fd, bool read, bool write);
    void rem(int fd);

private:
    typedef std::map<int, io_cb> cb_map;

private:
    void handle(void);

    uint32_t inline get_events(bool read, bool write);
    bool inline is_read(const uint32_t ev);
    bool inline is_write(const uint32_t ev);
    bool inline is_error(const uint32_t ev);

private:
    /* TODO: make this configurable */
    static const int timeout = 100;
    static const int epoll_max_events = 1000;

    int epfd;
    struct epoll_event epev[epoll_max_events];

    cb_map callbacks;
};

} /* namespace os_linux */
} /* namespace lixs */

#endif /* __LIXS_OS_LINUX_EPOLL_HH__ */

