#ifndef __LIXS_IOMUX_HH__
#define __LIXS_IOMUX_HH__


namespace lixs {

class ev_cb {
public:
    virtual void run(void) = 0;
};

class fd_cb {
public:
    struct fd_ev {
        bool read;
        bool write;

        fd_ev(bool _read, bool _write)
            : read(_read), write(_write)
        { };
    };

    virtual void handle(const fd_ev& events) = 0;
};

class iomux {
public:
    virtual void once(ev_cb& k) = 0;

    virtual void add(fd_cb& k, int fd, const fd_cb::fd_ev& ev) = 0;
    virtual void set(fd_cb& k, int fd, const fd_cb::fd_ev& ev) = 0;
    virtual void remove(int fd) = 0;

    virtual void handle(void) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_IOMUX_HH__ */

