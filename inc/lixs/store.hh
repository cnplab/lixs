#ifndef __LIXS_STORE_HH__
#define __LIXS_STORE_HH__

#include <string>


namespace lixs {

class store {
public:
    virtual const char* read(std::string key) = 0;
    virtual void write(std::string key, std::string val) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_STORE_HH__ */

