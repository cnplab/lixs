#include <lixs/mstore/database.hh>
#include <lixs/mstore/transaction.hh>
#include <lixs/util.hh>

#include <set>
#include <string>


lixs::mstore::transaction::transaction(unsigned int id, database& db)
    : db_access(db), id(id)
{
}

int lixs::mstore::transaction::create(cid_t cid, const std::string& path, bool& created)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq || rec.e.write_seq) {
        created = false;
    } else {
        ensure_branch(cid, path);
        register_with_parent(path);

        te.write_seq = rec.next_seq++;
        te.delete_seq = 0;
        get_parent_perms(path, te.perms);

        created = true;
    }

    return 0;
}

int lixs::mstore::transaction::read(cid_t cid, const std::string& path, std::string& val)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq) {
        if (!has_read_access(cid, te.perms)) {
            return EACCES;
        }

        val = te.value;
        return 0;
    } else if (!te.delete_seq && rec.e.write_seq) {
        if (!has_read_access(cid, rec.e.perms)) {
            return EACCES;
        }

        val = rec.e.value;
        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::transaction::update(cid_t cid, const std::string& path, const std::string& val)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq) {
        if (!has_write_access(cid, te.perms)) {
            return EACCES;
        }
    } else if (rec.e.write_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            return EACCES;
        }

        te.perms = rec.e.perms;
    } else {
        ensure_branch(cid, path);
        register_with_parent(path);

        get_parent_perms(path, te.perms);
    }

    te.value = val;
    te.write_seq = rec.next_seq++;
    te.delete_seq = 0;

    return 0;
}

int lixs::mstore::transaction::del(cid_t cid, const std::string& path)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq) {
        if (!has_write_access(cid, te.perms)) {
            return EACCES;
        }
    } else if (!te.delete_seq && rec.e.write_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            return EACCES;
        }
    } else {
        return ENOENT;
    }

    delete_branch(path);
    unregister_from_parent(path);

    te.delete_seq = rec.next_seq++;
    te.write_seq = 0;

    return 0;
}

int lixs::mstore::transaction::get_children(cid_t cid, const std::string& path, std::set<std::string>& resp)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq) {
        if (!has_read_access(cid, te.perms)) {
            return EACCES;
        }
    } else if (!te.delete_seq && rec.e.write_seq) {
        if (!has_read_access(cid, rec.e.perms)) {
            return EACCES;
        }
    } else {
        return ENOENT;
    }

    std::set<std::string>::iterator i;

    if (te.write_seq) {
        resp.insert(te.children.begin(), te.children.end());
    }

    if (rec.e.write_seq) {
        for (i = rec.e.children.begin(); i != rec.e.children.end(); i++) {
            if (!was_deleted(path + "/" + *i)) {
                resp.insert(*i);
            }
        }
    }

    return 0;
}

int lixs::mstore::transaction::get_perms(cid_t cid,
        const std::string& path, permission_list& perms)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq) {
        if (!has_read_access(cid, te.perms)) {
            return EACCES;
        }

        perms = te.perms;
        return 0;
    } else if (!te.delete_seq && rec.e.write_seq) {
        if (!has_read_access(cid, rec.e.perms)) {
            return EACCES;
        }

        perms = rec.e.perms;
        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::transaction::set_perms(cid_t cid,
        const std::string& path, const permission_list& perms)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq) {
        if (!has_write_access(cid, te.perms)) {
            return EACCES;
        }

        te.perms = perms;
        te.write_seq = rec.next_seq++;
        return 0;
    } else if (!te.delete_seq && rec.e.write_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            return EACCES;
        }

        te.value = rec.e.value;
        te.perms = perms;
        te.write_seq = rec.next_seq++;
        te.delete_seq = 0;
        return 0;
    } else {
        return ENOENT;
    }
}

void lixs::mstore::transaction::abort()
{
    std::set<std::string>::iterator it;

    for (it = records.begin(); it != records.end(); it++) {
        record& rec = db[*it];
        rec.te.erase(id);

        if (!rec.e.write_seq && rec.te.empty()) {
            db.erase(*it);
        }
    }
}

void lixs::mstore::transaction::merge(bool& success)
{
    success = can_merge();

    if (success) {
        do_merge();
    } else {
        abort();
    }
}

bool lixs::mstore::transaction::can_merge()
{
    std::set<std::string>::iterator it;

    for (it = records.begin(); it != records.end(); it++) {
        record& rec = db[*it];
        entry& te = rec.te[id];

        long int rec_seq = std::max(rec.e.write_seq, rec.e.delete_seq);

        if (rec_seq && (rec_seq > te.read_seq)) {
            return false;
        }

        long int te_seq = std::max(te.write_seq, te.delete_seq);

        if (te_seq && rec.e.read_seq > te_seq) {
            return false;
        }
    }

    return true;
}

void lixs::mstore::transaction::do_merge()
{
    long int merge_seq;

    std::string parent;
    std::string name;

    std::set<std::string>::iterator it;

    for (it = records.begin(); it != records.end(); it++) {
        record& rec = db[*it];
        entry& te = rec.te[id];

        merge_seq = rec.next_seq++;

        rec.e.read_seq = merge_seq;

        if (te.write_seq) {
            rec.e.value = te.value;
            rec.e.perms = te.perms;
            rec.e.write_seq = merge_seq;
            rec.e.delete_seq = 0;
        }

        if (te.delete_seq) {
            if (basename(*it, parent, name)) {
                db[parent].e.children.erase(name);
            }

            if (rec.te.size() == 1) {
                db.erase(*it);
            } else {
                rec.e.write_seq = 0;
                rec.e.delete_seq = merge_seq;
                rec.te.erase(id);
            }
        } else {
            rec.e.children.insert(te.children.begin(), te.children.end());
            rec.te.erase(id);
        }
    }
}

void lixs::mstore::transaction::register_with_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        record& rec = db[parent];
        entry& te = rec.te[id];

        if (!te.read_seq) {
            te.read_seq = rec.next_seq++;
            records.insert(parent);
        }

        te.children.insert(name);
    }
}

void lixs::mstore::transaction::unregister_from_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        record& rec = db[parent];
        tentry_map::iterator it;

        it = rec.te.find(id);
        if (it != rec.te.end()) {
            it->second.children.erase(name);
        }
    }
}

void lixs::mstore::transaction::ensure_branch(cid_t cid, const std::string& path)
{
    bool created;
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        record& rec = db[parent];

        if (rec.e.write_seq) {
            return;
        }

        if (rec.te[id].write_seq) {
            return;
        }

        create(cid, parent, created);
    }
}

void lixs::mstore::transaction::delete_branch(const std::string& path)
{
    std::set<std::string>::iterator it;
    std::set<std::string> children;

    record& rec = db[path];
    entry& te = rec.te[id];

    if (te.write_seq) {
        children.insert(te.children.begin(), te.children.end());
    }

    if (rec.e.write_seq) {
        children.insert(rec.e.children.begin(), rec.e.children.end());
    }

    /* Delete a subtree doesn't require access permissions. Only access to the
     * root node, so we delete as 0.
     */
    for (it = children.begin(); it != children.end(); it++) {
        del(0, path + "/" + *it);
    }
}

bool lixs::mstore::transaction::was_deleted(const std::string& path)
{
    record& rec = db[path];

    tentry_map::iterator it;

    it = rec.te.find(id);
    if (it != rec.te.end()) {
        return it->second.delete_seq != 0;
    } else {
        return false;
    }
}

void lixs::mstore::transaction::get_parent_perms(const std::string& path, permission_list& perms)
{
    std::string name;
    std::string parent;

    database::iterator it;
    tentry_map::iterator t_it;

    if (basename(path, parent, name)) {
        it = db.find(parent);
        if (it != db.end()) {
            record& rec = it->second;

            t_it = rec.te.find(id);
            if (t_it != rec.te.end()) {
                entry& te = t_it->second;

                if (te.write_seq) {
                    perms = te.perms;
                    return;
                }
            }

            if (rec.e.write_seq) {
                perms = rec.e.perms;
                return;
            }
        }
    }

    perms.clear();
    perms.push_back(permission(0, false, false));
}

