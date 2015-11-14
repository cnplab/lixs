#ifndef __LIXS_WATCH_HH__
#define __LIXS_WATCH_HH__

#include <string>


namespace lixs {

class watch_cb_k {
public:
    watch_cb_k(const std::string& path, const std::string& token)
        : path(path), token(token)
    { }

public:
    virtual void operator()(const std::string& path) = 0;

public:
    const std::string path;
    const std::string token;
};

} /* namespace lixs */

#endif /* __LIXS_WATCH_HH__ */

