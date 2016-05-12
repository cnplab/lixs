#ifndef __LIXS_MSTORE_SIMPLE_ACCESS_HH__
#define __LIXS_MSTORE_SIMPLE_ACCESS_HH__

#include <lixs/log/logger.hh>
#include <lixs/mstore/database.hh>

#include <set>
#include <string>


namespace lixs {
namespace mstore {

class simple_access : public db_access {
public:
    simple_access(database& db, log::logger& log);

    int create(cid_t cid, const std::string& path, bool& created);
    int read(cid_t cid, const std::string& path, std::string& val);
    int update(cid_t cid, const std::string& path, const std::string& val);
    int del(cid_t cid, const std::string& path);

    int get_children(cid_t cid, const std::string& path, std::set<std::string>& resp);

    int get_perms(cid_t cid, const std::string& path, permission_list& perms);
    int set_perms(cid_t cid, const std::string& path, const permission_list& perms);

private:
    void register_with_parent(const std::string& path);
    void unregister_from_parent(const std::string& path);
    void ensure_branch(cid_t cid, const std::string& path);
    void delete_branch(const std::string& path, record& rec);
    void get_parent_perms(const std::string& path, permission_list& perms);
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_SIMPLE_ACCESS_HH__ */

