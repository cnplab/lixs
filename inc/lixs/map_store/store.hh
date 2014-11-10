#ifndef __LIXS_MAP_STORE_STORE_HH__
#define __LIXS_MAP_STORE_STORE_HH__

#include <lixs/store.hh>
#include <lixs/map_store/record.hh>
#include <lixs/map_store/transaction.hh>

#include <map>
#include <string>
#include <sys/time.h>


namespace lixs {
namespace map_store {

class store : public lixs::store {
public:
    store(void);
    ~store();

    void branch(unsigned int& id);
    void merge(unsigned int id, bool& success);
    void abort(unsigned int id);

    int create(int id, std::string key, bool& created);
    int read(int id, std::string key, std::string& val);
    int update(int id, std::string key, std::string val);
    int del(int id, std::string key);

    int get_childs(std::string key, const char* resp[], int nresp);

private:
    std::map<std::string, record> data;
    std::map<int, transaction> ltrans;
    unsigned int next_id;

    void ensure_parents(int id, const std::string& key);
    void delete_subtree(int id, const std::string& key);
};

} /* namespace map_store */
} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_STORE_HH__ */

