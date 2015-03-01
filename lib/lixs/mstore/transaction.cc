#include <lixs/mstore/database.hh>
#include <lixs/mstore/transaction.hh>
#include <lixs/util.hh>

#include <set>
#include <string>


lixs::mstore::transaction::transaction(unsigned int id, database& db)
    : db_access(db), id(id)
{
}

int lixs::mstore::transaction::create(const std::string& path, bool& created)
{
    ensure_branch(path);

    record& rec = db[path];

    if (rec.e.write_seq) {
        created = false;
    } else {
        entry& te = rec.te[id];

        if (!te.init_seq) {
            te.init_seq = rec.next_seq++;
            records.insert(path);
        }

        if (te.write_seq) {
            created = false;
        } else {
            register_with_parent(path);

            te.write_seq = rec.next_seq++;
            te.delete_seq = 0;

            created = true;
        }
    }

    return 0;
}

int lixs::mstore::transaction::read(const std::string& path, std::string& val)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.init_seq) {
        te.init_seq = rec.next_seq++;
        records.insert(path);
    }

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
    }

    if (te.write_seq) {
        val = te.value;
        return 0;
    } else if (rec.e.write_seq) {
        val = rec.e.value;
        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::transaction::update(const std::string& path, const std::string& val)
{
    ensure_branch(path);

    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.init_seq) {
        te.init_seq = rec.next_seq++;
        records.insert(path);
    }

    if (!te.write_seq) {
        register_with_parent(path);
    }

    te.value = val;
    te.write_seq = rec.next_seq++;
    te.delete_seq = 0;

    return 0;
}

int lixs::mstore::transaction::del(const std::string& path)
{
    database::iterator it;

    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.init_seq) {
        te.init_seq = rec.next_seq++;
        records.insert(path);
    }

    if (te.write_seq || rec.e.write_seq) {
        delete_branch(path);

        if (te.write_seq) {
            unregister_from_parent(path);
        }

        te.delete_seq = rec.next_seq++;
        te.write_seq = 0;

        return 0;
    } else {
        if (!te.read_seq) {
            te.read_seq = rec.next_seq++;
        }

        return ENOENT;
    }
}

int lixs::mstore::transaction::get_children(const std::string& path, std::set<std::string>& resp)
{
    record& rec = db[path];
    entry& te = rec.te[id];

    if (!te.init_seq) {
        te.init_seq = rec.next_seq++;
        records.insert(path);
    }

    if (!te.read_seq) {
        te.read_seq = rec.next_seq++;
    }

    std::set<std::string>::iterator i;

    if (!(te.write_seq || rec.e.write_seq)) {
        return ENOENT;
    }

    if (te.write_seq) {
        for (i = te.children.begin(); i != te.children.end(); i++) {
            resp.insert(*i);
        }
    }

    if (rec.e.write_seq) {
        for (i = rec.e.children.begin(); i != rec.e.children.end(); i++) {
            if (!db[path + *i].te[id].delete_seq) {
                resp.insert(*i);
            }
        }
    }

    return 0;
}

void lixs::mstore::transaction::abort()
{
    std::set<std::string>::iterator it;

    for (it = records.begin(); it != records.end(); it++) {
        record& rec = db[*it];
        rec.te.erase(id);

        if (!rec.e.write_seq && rec.te.size() == 0) {
            db.erase(*it);
        }
    }
}

void lixs::mstore::transaction::merge(bool& success)
{
    success = can_merge();

    if (success) {
        _merge();
    } else {
        abort();
    }
}

bool lixs::mstore::transaction::can_merge()
{
    std::set<std::string>::iterator it;

    for (it = records.begin(); it != records.end(); it++) {
        record& rec = db[*it];
        unsigned int read_seq = rec.te[id].read_seq;

        if (read_seq && (rec.e.write_seq > read_seq || rec.e.delete_seq > read_seq)) {
            return false;
        }
    }

    return true;
}

void lixs::mstore::transaction::_merge()
{
    std::string parent;
    std::string name;

    std::set<std::string>::iterator it;

    for (it = records.begin(); it != records.end(); it++) {
        record& rec = db[*it];
        entry& te = rec.te[id];

        if (te.write_seq) {
            rec.e.value = te.value;
            rec.e.write_seq = te.write_seq;
        }

        if (te.delete_seq) {
            if (basename(*it, parent, name)) {
                db[parent].e.children.erase(name);
            }

            if (rec.te.size() == 1) {
                db.erase(*it);
            } else {
                rec.e.delete_seq = te.delete_seq;
                rec.e.write_seq = 0;
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
        db[parent].te[id].children.insert(name);

        if (!db[parent].te[id].init_seq) {
            records.insert(parent);
        }
    }
}

void lixs::mstore::transaction::unregister_from_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        db[parent].te[id].children.erase(name);
    }
}

void lixs::mstore::transaction::ensure_branch(const std::string& path)
{
    size_t pos;
    bool created;

    pos = path.rfind('/');
    if (pos != std::string::npos) {
        std::string parent = path.substr(0, pos);
        record& rec = db[parent];

        if (!(rec.e.write_seq || rec.te[id].write_seq)) {
            create(parent, created);
        }
    }
}

void lixs::mstore::transaction::delete_branch(const std::string& path)
{
    std::set<std::string>::iterator it;
    std::set<std::string> children;

    record& rec = db[path];
    entry& te = rec.te[id];

    children.insert(te.children.begin(), te.children.end());

    if (rec.e.write_seq) {
        children.insert(rec.e.children.begin(), rec.e.children.end());
    }

    for (it = children.begin(); it != children.end(); it++) {
        del(path + "/" + *it);
    }
}

