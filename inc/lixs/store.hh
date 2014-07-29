#ifndef __LIXS_STORE_HH__
#define __LIXS_STORE_HH__

#include <string>


namespace lixs {

class store {
public:
    virtual const char* read(std::string key) = 0;
    virtual void write(std::string key, std::string val) = 0;
    virtual void del(std::string key) = 0;

    virtual void branch(int id) = 0;
    virtual bool merge(int id) = 0;
    virtual void abort(int id) = 0;
    virtual const char* read(int id, std::string key) = 0;
    virtual void write(int id, std::string key, std::string val) = 0;
    virtual void del(int id, std::string key) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_STORE_HH__ */

