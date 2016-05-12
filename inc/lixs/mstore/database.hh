#ifndef __LIXS_MSTORE_DATABASE_HH__
#define __LIXS_MSTORE_DATABASE_HH__

#include <lixs/log/logger.hh>
#include <lixs/permissions.hh>

#include <map>
#include <set>
#include <string>


namespace lixs {
namespace mstore {

class entry {
public:
    entry ()
        : write_seq(0), delete_seq(0), write_children_seq(0)
    { }

    /* Data */
    std::string value;
    permission_list perms;

    /* Metadata for tree management */
    std::set<std::string> children;

    /* Metadata for transaction management */
    long int write_seq;
    long int delete_seq;
    long int write_children_seq;
};

class tentry {
public:
    tentry ()
        : init_seq(0), init_valid(false), read_seq(0), write_seq(0), delete_seq(0),
        read_children_seq(0)
    { }

    /* Data */
    std::string value;
    permission_list perms;

    /* Metadata for tree management */
    std::set<std::string> children;
    std::set<std::string> children_add;
    std::set<std::string> children_rem;

    /* Metadata for transaction management */
    long int init_seq;
    bool init_valid;

    long int read_seq;
    long int write_seq;
    long int delete_seq;

    long int read_children_seq;
};

typedef std::map<unsigned int, tentry> tentry_map;


class record {
public:
    record()
        : next_seq(1)
    { }

    entry e;
    tentry_map te;

    long int next_seq;
};

typedef std::map<std::string, record> database;


class db_access {
public:
    db_access(database& db, log::logger& log)
        : db(db), log(log)
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

    log::logger& log;
};


bool has_read_access(cid_t cid, const permission_list& perms);
bool has_write_access(cid_t cid, const permission_list& perms);

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_DATABASE_HH__ */

