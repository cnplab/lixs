#ifndef __LIXS_MSTORE_SIMPLE_ACCESS_HH__
#define __LIXS_MSTORE_SIMPLE_ACCESS_HH__

#include <lixs/mstore/database.hh>

#include <list>
#include <map>
#include <set>
#include <string>


namespace lixs {
namespace mstore {

class simple_access : public db_access {
public:
    simple_access(database& db);

    int create(const std::string& path, bool& created);
    int read(const std::string& path, std::string& val);
    int update(const std::string& path, const std::string& val);
    int del(const std::string& path);

    int get_children(const std::string& path, std::list<std::string>& resp);

private:
    void register_with_parent(const std::string& path);
    void unregister_from_parent(const std::string& path);
    void ensure_branch(const std::string& path);
    void delete_branch(const std::string& path);
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_SIMPLE_ACCESS_HH__ */

