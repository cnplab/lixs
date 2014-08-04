#ifndef __LIXS_EVENTS_HH__
#define __LIXS_EVENTS_HH__

#include <string>


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

class watch_cb_k {
public:
    watch_cb_k(void)
        : path("/"), token("")
    { };

    watch_cb_k(const char* path, const char* token)
        : path(path), token(token)
    { };

    watch_cb_k(const std::string& path, const std::string& token)
        : path(path), token(token)
    { };

    virtual void operator()(const std::string& path) = 0;

    std::string path;
    std::string token;
};

} /* namespace lixs */

#endif /* __LIXS_EVENTS_HH__ */

