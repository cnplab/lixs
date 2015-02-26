#ifndef __LIXS_MSTORE_STORE_HH__
#define __LIXS_MSTORE_STORE_HH__

#include <lixs/store.hh>
#include <lixs/mstore/database.hh>
#include <lixs/mstore/simple_access.hh>
#include <lixs/mstore/transaction.hh>

#include <map>
#include <set>
#include <string>


namespace lixs {
namespace mstore {

class store : public lixs::store {
public:
    store(void);
    ~store();

    void branch(unsigned int& tid);
    int merge(unsigned int tid, bool& success);
    int abort(unsigned int tid);

    int create(unsigned int tid, std::string path, bool& created);
    int read(unsigned int tid, std::string key, std::string& val);
    int update(unsigned int tid, std::string key, std::string val);
    int del(unsigned int tid, std::string key);

    int get_children(unsigned int tid, std::string key, std::set<std::string>& resp);

private:
    typedef std::map<unsigned int, transaction> transaction_db;


    database db;

    simple_access access;

    unsigned int next_tid;
    transaction_db trans;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_STORE_HH__ */

