#ifndef __LIXS_IOMUX_HH__
#define __LIXS_IOMUX_HH__


namespace lixs {

struct fd_cb_k {
    fd_cb_k(void)
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
    virtual void add(fd_cb_k& cb) = 0;
    virtual void set(fd_cb_k& cb) = 0;
    virtual void remove(fd_cb_k& cb) = 0;

    virtual void handle(void) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_IOMUX_HH__ */

