#ifndef __LIXS_IOMUX_HH__
#define __LIXS_IOMUX_HH__

#include <lixs/event_mgr.hh>


namespace lixs {

struct io_cb {
    io_cb(void)
        : fd(-1), ev_read(false), ev_write(false)
    { }

    /* FIXME: Provide information about errors (e.g. EPOLLERR) */
    virtual void operator() (bool ev_read, bool ev_write) = 0;

    int fd;
    bool ev_read;
    bool ev_write;
};

class iomux {
public:
    iomux(event_mgr& emgr)
        : emgr(emgr)
    { }

public:
    virtual void add(io_cb& cb) = 0;
    virtual void set(io_cb& cb) = 0;
    virtual void remove(io_cb& cb) = 0;

protected:
    event_mgr& emgr;
};

} /* namespace lixs */

#endif /* __LIXS_IOMUX_HH__ */

