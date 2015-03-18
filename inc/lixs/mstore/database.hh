#ifndef __LIXS_MSTORE_DATABASE_HH__
#define __LIXS_MSTORE_DATABASE_HH__

#include <lixs/permissions.hh>

#include <map>
#include <set>
#include <string>


namespace lixs {
namespace mstore {

class entry {
public:
    entry ()
        : init_seq(0), read_seq(0), write_seq(0), delete_seq(0)
    { }

    std::string value;
    std::set<std::string> children;

    permission_list perms;

    long int init_seq;
    long int read_seq;
    long int write_seq;
    long int delete_seq;
};

class record {
public:
    record()
        : next_seq(1)
    { }

    long int next_seq;

    entry e;
    std::map<unsigned int, entry> te;
};

typedef std::map<std::string, record> database;

class db_access {
public:
    db_access(database& db)
        : db(db)
    { }

    virtual int create(cid_t cid, const std::string& path, bool& created) = 0;
    virtual int read(cid_t cid, const std::string& path, std::string& val) = 0;
    virtual int update(cid_t cid, const std::string& path, const std::string& val) = 0;
    virtual int del(cid_t cid, const std::string& path) = 0;

    virtual int get_children(cid_t cid, const std::string& path, std::set<std::string>& resp) = 0;

    virtual int get_perms(cid_t cid, const std::string& path, permission_list& perms) = 0;
    virtual int set_perms(cid_t cid, const std::string& path, const permission_list& perms) = 0;

protected:
    database& db;
};

bool has_read_access(cid_t cid, const permission_list& perms);
bool has_write_access(cid_t cid, const permission_list& perms);

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_DATABASE_HH__ */

