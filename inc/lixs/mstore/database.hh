#ifndef __LIXS_MSTORE_DATABASE_HH__
#define __LIXS_MSTORE_DATABASE_HH__

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

    virtual int create(const std::string& path, bool& created) = 0;
    virtual int read(const std::string& path, std::string& val) = 0;
    virtual int update(const std::string& path, const std::string& val) = 0;
    virtual int del(const std::string& path) = 0;

    virtual int get_children(const std::string& path, std::set<std::string>& resp) = 0;

protected:
    database& db;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_DATABASE_HH__ */

