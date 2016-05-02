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

    int create(cid_t cid, const std::string& path, bool& created);
    int read(cid_t cid, const std::string& path, std::string& val);
    int update(cid_t cid, const std::string& path, const std::string& val);
    int del(cid_t cid, const std::string& path);

    int get_children(cid_t cid, const std::string& path, std::set<std::string>& resp);

    int get_perms(cid_t cid, const std::string& path, permission_list& perms);
    int set_perms(cid_t cid, const std::string& path, const permission_list& perms);

private:
    bool can_merge();
    void do_merge();

    void register_with_parent(const std::string& path);
    void unregister_from_parent(const std::string& path);
    void ensure_branch(cid_t cid, const std::string& path);
    void delete_branch(const std::string& path, tentry& te);
    void get_parent_perms(const std::string& path, permission_list& perms);
    tentry& get_tentry(const std::string& path, record& rec);
    void fetch_tentry_data(tentry& te, record& rec);
    void fetch_tentry_children(tentry& te, record& rec);

    unsigned int id;
    std::set<std::string> records;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_TRANSACTION_HH__ */

