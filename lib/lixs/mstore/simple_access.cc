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
    record& rec = db[path];

    if (rec.e.write_seq) {
        rec.e.read_seq = rec.next_seq++;
        created = false;
    } else {
        ensure_branch(cid, path);
        register_with_parent(path);

        rec.e.write_seq = rec.next_seq++;
        rec.e.delete_seq = 0;
        get_parent_perms(path, rec.e.perms);

        created = true;
    }

    return 0;
}

int lixs::mstore::simple_access::read(cid_t cid, const std::string& path, std::string& val)
{
    database::iterator it;

    it = db.find(path);
    if (it == db.end()) {
        return ENOENT;
    }

    record& rec = it->second;

    rec.e.read_seq = rec.next_seq++;

    if (rec.e.write_seq) {
        if (!has_read_access(cid, rec.e.perms)) {
            return EACCES;
        }

        val = rec.e.value;
        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::update(cid_t cid, const std::string& path, const std::string& val)
{
    record& rec = db[path];

    if (rec.e.write_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            rec.e.read_seq = rec.next_seq++;
            return EACCES;
        }
    } else {
        ensure_branch(cid, path);
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
    if (it == db.end()) {
        return ENOENT;
    }

    record& rec = it->second;

    if (rec.e.write_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            rec.e.read_seq = rec.next_seq++;
            return EACCES;
        }

        delete_branch(path);
        unregister_from_parent(path);

        if (rec.te.empty()) {
            db.erase(it);
        } else {
            rec.e.write_seq = 0;
            rec.e.delete_seq = rec.next_seq++;
        }
        return 0;
    } else {
        rec.e.read_seq = rec.next_seq++;
        return ENOENT;
    }
}

int lixs::mstore::simple_access::get_children(cid_t cid,
        const std::string& path, std::set<std::string>& resp)
{
    database::iterator it;

    it = db.find(path);
    if (it == db.end()) {
        return ENOENT;
    }

    record& rec = it->second;

    rec.e.read_seq = rec.next_seq++;

    if (rec.e.write_seq) {
        if (!has_read_access(cid, rec.e.perms)) {
            return EACCES;
        }

        resp.insert(rec.e.children.begin(), rec.e.children.end());

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
    if (it == db.end()) {
        return ENOENT;
    }

    record& rec = it->second;

    rec.e.read_seq = rec.next_seq++;

    if (rec.e.write_seq) {
        if (!has_read_access(cid, rec.e.perms)) {
            return EACCES;
        }

        perms = rec.e.perms;
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
    if (it == db.end()) {
        return ENOENT;
    }

    record& rec = it->second;

    if (rec.e.write_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            rec.e.read_seq = rec.next_seq++;
            return EACCES;
        }

        it->second.e.perms = perms;
        rec.e.write_seq = rec.next_seq++;

        return 0;
    } else {
        rec.e.read_seq = rec.next_seq++;
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
    bool created;
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        create(cid, parent, created);
    }
}

void lixs::mstore::simple_access::delete_branch(const std::string& path)
{
    record& rec = db[path];

    /* Delete a subtree doesn't require access permissions. Only access to the
     * root node, so we delete as 0.
     */
    /* del will eventually call unregister_from_parent which will update
     * rec.e.children, deleting the element we're currently iterating on,
     * therefore don't use an iterator here.
     */
    while (!rec.e.children.empty()) {
        del(0, path + "/" + *(rec.e.children.begin()));
    }
}

void lixs::mstore::simple_access::get_parent_perms(const std::string& path, permission_list& perms)
{
    std::string name;
    std::string parent;
    database::iterator it;

    if (basename(path, parent, name)) {
        it = db.find(parent);
        if (it != db.end() && it->second.e.write_seq) {
            perms = it->second.e.perms;
            return;
        }
    }

    perms.clear();
    perms.push_back(permission(0, false, false));
}

