#ifndef __LIXS_STORE_HH__
#define __LIXS_STORE_HH__

#include <string>


namespace lixs {

class store {
public:
    virtual void branch(unsigned int& id) = 0;
    virtual void merge(unsigned int id, bool& success) = 0;
    virtual void abort(unsigned int id) = 0;

    virtual int create(int id, std::string key, bool& created) = 0;
    virtual int read(int id, std::string key, std::string& val) = 0;
    virtual int update(int id, std::string key, std::string val) = 0;
    virtual int del(int id, std::string key) = 0;

    virtual int get_childs(std::string key, const char* resp[], int nresp) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_STORE_HH__ */

