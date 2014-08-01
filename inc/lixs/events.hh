#ifndef __LIXS_EVENTS_HH__
#define __LIXS_EVENTS_HH__


namespace lixs {

class ev_cb_k {
public:
    virtual void operator()(void) = 0;
};

class fd_cb_k {
public:
    fd_cb_k(void)
        : fd(-1), ev_read(false), ev_write(false)
    { };

    virtual void operator() (bool ev_read, bool ev_write) = 0;

    int fd;
    bool ev_read;
    bool ev_write;
};

} /* namespace lixs */

#endif /* __LIXS_EVENTS_HH__ */

