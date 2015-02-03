#ifndef __LIXS_EVENTS_HH__
#define __LIXS_EVENTS_HH__

#include <string>


namespace lixs {

struct fd_cb_k {
    fd_cb_k(void)
        : fd(-1), ev_read(false), ev_write(false)
    { };

    /* FIXME: Provide information about errors (e.g. EPOLLERR) */
    virtual void operator() (bool ev_read, bool ev_write) = 0;

    int fd;
    bool ev_read;
    bool ev_write;
};

struct watch_cb_k {
    watch_cb_k(const std::string& path, const std::string& token, bool relative)
        : path(path), token(token), relative(relative)
    { };

    virtual void operator()(const std::string& path) = 0;

    std::string path;
    std::string token;
    bool relative;
};

} /* namespace lixs */

#endif /* __LIXS_EVENTS_HH__ */

