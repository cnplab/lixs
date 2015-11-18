#ifndef __LIXS_IOMUX_HH__
#define __LIXS_IOMUX_HH__

#include <lixs/event_mgr.hh>

#include <functional>


namespace lixs {

typedef std::function<void(bool, bool, bool)> io_cb;

class iomux {
public:
    iomux(event_mgr& emgr)
        : emgr(emgr)
    { }

public:
    virtual void add(int fd, bool read, bool write, io_cb cb) = 0;
    virtual void set(int fd, bool read, bool write) = 0;
    virtual void rem(int fd) = 0;

protected:
    event_mgr& emgr;
};

} /* namespace lixs */

#endif /* __LIXS_IOMUX_HH__ */

