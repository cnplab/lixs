#include <lixs/mstore/database.hh>
#include <lixs/permissions.hh>
#include <lixs/mstore/simple_access.hh>
#include <lixs/util.hh>

#include <set>
#include <string>


lixs::mstore::simple_access::simple_access(database& db)
    : db_access(db)
{
}

int lixs::mstore::simple_access::create(cid_t cid, const std::string& path, bool& created)
{
    ensure_branch(cid, path);

    record& rec = db[path];

    if (rec.e.write_seq) {
        created = false;
    } else {
        rec.e.write_seq = rec.next_seq++;
        get_parent_perms(path, rec.e.perms);

        register_with_parent(path);
        created = true;
    }

    return 0;
}

int lixs::mstore::simple_access::read(cid_t cid, const std::string& path, std::string& val)
{
    database::iterator it;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        if (!has_read_access(cid, it->second.e.perms)) {
            return EACCES;
        }

        val = it->second.e.value;
        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::update(cid_t cid, const std::string& path, const std::string& val)
{
    record& rec = db[path];

    if (!has_write_access(cid, rec.e.perms)) {
        return EACCES;
    }

    ensure_branch(cid, path);

    if (!rec.e.write_seq) {
        register_with_parent(path);

        get_parent_perms(path, rec.e.perms);
    }

    rec.e.value = val;
    rec.e.write_seq = rec.next_seq++;
    rec.e.delete_seq = 0;

    return 0;
}

int lixs::mstore::simple_access::del(cid_t cid, const std::string& path)
{
    database::iterator it;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        if (!has_write_access(cid, it->second.e.perms)) {
            return EACCES;
        }

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

int lixs::mstore::simple_access::get_children(cid_t cid,
        const std::string& path, std::set<std::string>& resp)
{
    database::iterator it;
    std::set<std::string>::iterator i;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        if (!has_read_access(cid, it->second.e.perms)) {
            return EACCES;
        }

        for (i = it->second.e.children.begin(); i != it->second.e.children.end(); i++) {
            resp.insert(*i);
        }

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::get_perms(cid_t cid,
        const std::string& path, permission_list& perms)
{
    database::iterator it;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        if (!has_read_access(cid, it->second.e.perms)) {
            return EACCES;
        }

        perms = it->second.e.perms;

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::set_perms(cid_t cid,
        const std::string& path, const permission_list& perms)
{
    database::iterator it;

    it = db.find(path);
    if (it != db.end() && it->second.e.write_seq) {
        if (!has_write_access(cid, it->second.e.perms)) {
            return EACCES;
        }

        it->second.e.perms = perms;

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

void lixs::mstore::simple_access::ensure_branch(cid_t cid, const std::string& path)
{
    size_t pos;
    bool created;

    pos = path.rfind('/');
    if (pos != std::string::npos) {
        std::string parent = path.substr(0, pos);
        record& rec = db[parent];

        if (!rec.e.write_seq) {
            create(cid, parent, created);
        }
    }
}

void lixs::mstore::simple_access::delete_branch(const std::string& path)
{
    std::set<std::string>::iterator it;

    record& rec = db[path];

    /* Delete a subtree doesn't require access permissions. Only access to the
     * root node, so we delete as 0.
     */
    for (it = rec.e.children.begin(); it != rec.e.children.end(); it++) {
        del(0, path + "/" + *it);
    }
}

void lixs::mstore::simple_access::get_parent_perms(const std::string& path, permission_list& perms)
{
    size_t pos;
    database::iterator it;

    pos = path.rfind('/');
    if (pos != std::string::npos) {
        std::string parent = path.substr(0, pos);

        it = db.find(parent);
        if (it != db.end() && it->second.e.write_seq) {
            perms = it->second.e.perms;
            return;
        }
    }

    perms.clear();
    perms.push_back(permission(0, false, false));
}

