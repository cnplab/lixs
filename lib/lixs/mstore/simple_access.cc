#include <lixs/mstore/database.hh>
#include <lixs/mstore/simple_access.hh>
#include <lixs/mstore/util.hh>

#include <map>
#include <set>
#include <string>


lixs::mstore::simple_access::simple_access(database& db)
    : db_access(db)
{
}

int lixs::mstore::simple_access::create(const std::string& path, bool& created)
{
    ensure_branch(path);

    record& rec = db[path];

    if (rec.e.write_seq) {
        created = false;
    } else {
        rec.e.write_seq = rec.next_seq++;
        register_with_parent(path);
        created = true;
    }

    return 0;
}

int lixs::mstore::simple_access::read(const std::string& path, std::string& val)
{
    database::iterator it;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        val = it->second.e.value;
        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::update(const std::string& path, const std::string& val)
{
    ensure_branch(path);

    record& rec = db[path];

    if (!rec.e.write_seq) {
        register_with_parent(path);
    }

    rec.e.value = val;
    rec.e.write_seq = rec.next_seq++;
    rec.e.delete_seq = 0;

    return 0;
}

int lixs::mstore::simple_access::del(const std::string& path)
{
    database::iterator it;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        delete_branch(path);

        unregister_from_parent(path);

        if (it->second.te.size() == 0) {
            db.erase(path);
        } else {
            it->second.e.write_seq = 0;
            it->second.e.delete_seq = it->second.next_seq++;
        }
        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::get_children(const std::string& path, std::set<std::string>& resp)
{
    database::iterator it;
    std::set<std::string>::iterator i;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        for (i = it->second.e.children.begin(); i != it->second.e.children.end(); i++) {
            resp.insert(*i);
        }

        return 0;
    } else {
        return ENOENT;
    }
}

void lixs::mstore::simple_access::register_with_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        db[parent].e.children.insert(name);
    }
}

void lixs::mstore::simple_access::unregister_from_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        db[parent].e.children.erase(name);
    }
}

void lixs::mstore::simple_access::ensure_branch(const std::string& path)
{
    size_t pos;
    bool created;
    database::iterator it;

    pos = path.rfind('/');
    if (pos != std::string::npos) {
        std::string parent = path.substr(0, pos);
        record& rec = db[parent];

        if (!rec.e.write_seq) {
            create(parent, created);
        }
    }
}

void lixs::mstore::simple_access::delete_branch(const std::string& path)
{
    std::set<std::string>::iterator it;

    record& rec = db[path];

    for (it = rec.e.children.begin(); it != rec.e.children.end(); it++) {
        del(path + "/" + *it);
    }
}

