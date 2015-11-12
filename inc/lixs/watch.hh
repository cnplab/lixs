#ifndef __LIXS_WATCH_HH__
#define __LIXS_WATCH_HH__

#include <string>


namespace lixs {

class watch_cb_k {
public:
    watch_cb_k(const std::string& path, const std::string& token, bool relative)
        : path(path), token(token), relative(relative)
    { }

public:
    virtual void operator()(const std::string& path) = 0;

public:
    const std::string path;
    const std::string token;
    const bool relative;
};

} /* namespace lixs */

#endif /* __LIXS_WATCH_HH__ */

