#ifndef __LIXS_MSTORE_TRANSACTION_HH__
#define __LIXS_MSTORE_TRANSACTION_HH__

#include <lixs/mstore/database.hh>

#include <set>
#include <string>


namespace lixs {
namespace mstore {

class transaction : public db_access {
public:
    transaction(unsigned int id, database& db);

    void abort();
    void merge(bool& success);

    int create(const std::string& path, bool& created);
    int read(const std::string& path, std::string& val);
    int update(const std::string& path, const std::string& val);
    int del(const std::string& path);

    int get_children(const std::string& path, std::set<std::string>& resp);

private:
    bool can_merge();
    void _merge();

    void register_with_parent(const std::string& path);
    void unregister_from_parent(const std::string& path);
    void ensure_branch(const std::string& path);
    void delete_branch(const std::string& path);

    unsigned int id;
    std::set<std::string> records;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_TRANSACTION_HH__ */

