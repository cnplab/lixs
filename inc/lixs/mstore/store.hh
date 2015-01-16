#ifndef __LIXS_MSTORE_STORE_HH__
#define __LIXS_MSTORE_STORE_HH__

#include <lixs/store.hh>
#include <lixs/mstore/database.hh>
#include <lixs/mstore/simple_access.hh>
#include <lixs/mstore/transaction.hh>

#include <list>
#include <map>
#include <string>


namespace lixs {
namespace mstore {

class store : public lixs::store {
public:
    store(void);
    ~store();

    void branch(unsigned int& tid);
    void merge(unsigned int tid, bool& success);
    void abort(unsigned int tid);

    int create(unsigned int tid, std::string path, bool& created);
    int read(unsigned int tid, std::string key, std::string& val);
    int update(unsigned int tid, std::string key, std::string val);
    int del(unsigned int tid, std::string key);

    void get_children(std::string key, std::list<std::string>& resp);

private:
    database db;

    simple_access access;

    unsigned int next_tid;
    std::map<unsigned int, transaction> trans;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_STORE_HH__ */

