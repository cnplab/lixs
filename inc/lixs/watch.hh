#ifndef __LIXS_WATCH_HH__
#define __LIXS_WATCH_HH__

#include <string>


namespace lixs {

struct watch_cb_k {
    watch_cb_k(const std::string& path, const std::string& token, bool relative)
        : path(path), token(token), relative(relative)
    { }

    virtual void operator()(const std::string& path) = 0;

    std::string path;
    std::string token;
    bool relative;
};

} /* namespace lixs */

#endif /* __LIXS_WATCH_HH__ */

