#ifndef __LIXS_MSTORE_STORE_HH__
#define __LIXS_MSTORE_STORE_HH__

#include <lixs/log/logger.hh>
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
    store(log::logger& log);
    ~store();

    void branch(unsigned int& tid);
    int merge(unsigned int tid, bool& success);
    int abort(unsigned int tid);

    int create(cid_t cid, unsigned int tid,
            std::string path, bool& created);
    int read(cid_t cid, unsigned int tid,
            std::string key, std::string& val);
    int update(cid_t cid, unsigned int tid,
            std::string key, std::string val);
    int del(cid_t cid, unsigned int tid,
            std::string key);

    int get_children(cid_t cid, unsigned int tid,
            std::string key, std::set<std::string>& resp);

    int get_perms(cid_t cid, unsigned int tid,
            std::string path, permission_list& perms);
    int set_perms(cid_t cid, unsigned int tid,
            std::string path, const permission_list& perms);

private:
    typedef std::map<unsigned int, transaction> transaction_db;


    database db;

    simple_access access;

    unsigned int next_tid;
    transaction_db trans;

    log::logger& log;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_STORE_HH__ */

