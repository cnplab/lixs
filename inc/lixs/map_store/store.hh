#ifndef __LIXS_MAP_STORE_STORE_HH__
#define __LIXS_MAP_STORE_STORE_HH__

#include <lixs/store.hh>
#include <lixs/map_store/database.hh>
#include <lixs/map_store/transaction.hh>

#include <list>
#include <map>
#include <string>


namespace lixs {
namespace map_store {

class store : public lixs::store {
public:
    store(void);
    ~store();

    void branch(unsigned int& id);
    int merge(unsigned int id, bool& success);
    int abort(unsigned int id);

    int create(unsigned int id, std::string key, bool& created);
    int read(unsigned int id, std::string key, std::string& val);
    int update(unsigned int id, std::string key, std::string val);
    int del(unsigned int id, std::string key);

    int get_children(unsigned int id, std::string key, std::list<std::string>& resp);

private:
    database data;
    std::map<int, transaction> ltrans;
    unsigned int next_id;

    void ensure_parents(unsigned int id, const std::string& key);
    void delete_subtree(unsigned int id, const std::string& key);
};

} /* namespace map_store */
} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_STORE_HH__ */

