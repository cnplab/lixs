#ifndef __LIXS_STORE_HH__
#define __LIXS_STORE_HH__

#include <set>
#include <string>


namespace lixs {

class store {
public:
    virtual void branch(unsigned int& tid) = 0;
    virtual void merge(unsigned int tid, bool& success) = 0;
    virtual void abort(unsigned int tid) = 0;

    virtual int create(unsigned int tid, std::string path, bool& created) = 0;
    virtual int read(unsigned int tid, std::string path, std::string& val) = 0;
    virtual int update(unsigned int tid, std::string path, std::string val) = 0;
    virtual int del(unsigned int tid, std::string path) = 0;

    virtual int get_children(unsigned int tid, std::string path, std::set<std::string>& resp) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_STORE_HH__ */

