#include <lixs/log/logger.hh>
#include <lixs/mstore/database.hh>
#include <lixs/permissions.hh>
#include <lixs/mstore/simple_access.hh>
#include <lixs/util.hh>

#include <set>
#include <string>


lixs::mstore::simple_access::simple_access(database& db, log::logger& log)
    : db_access(db, log)
{
}

int lixs::mstore::simple_access::create(cid_t cid, const std::string& path, bool& created)
{
    /* Here we can use the array operator since the entry either exists or will be created. */
    record& rec = db[path];

    if (rec.e.write_seq > rec.e.delete_seq) {
        created = false;
    } else {
        /* Ensure the parent branch exists and is valid and register the new entry. */
        ensure_branch(cid, path);
        register_with_parent(path);

        /* Set data to the defaults: empty value and the permissions inherited from parent. */
        /* If the entry is being used in a transaction we can get here with a non-empty value,
         * so be sure to reset it.
         */
        rec.e.value = "";
        get_parent_perms(path, rec.e.perms);

        /* Finally mark the entry as written and therefore as valid. */
        rec.e.write_seq = rec.next_seq++;

        created = true;
    }

    return 0;
}

int lixs::mstore::simple_access::read(cid_t cid, const std::string& path, std::string& val)
{
    /* On reading we can't create a new entry, so don't use the array operator. */
    database::iterator it;

    it = db.find(path);
    if (it == db.end()) {
        return ENOENT;
    }

    /* Get a reference for code clarity. */
    record& rec = it->second;

    /* If the entry is used in a transaction we might get here with an invalid entry, verify. */
    if (rec.e.write_seq > rec.e.delete_seq) {
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
    /* Here we can use the array operator since the entry either exists or will be created. */
    record& rec = db[path];

    if (rec.e.write_seq > rec.e.delete_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            return EACCES;
        }
    } else {
        /* Creating a new entry here: ensure the parent branch exists and is valid and register the
         * new entry.
         */
        ensure_branch(cid, path);
        register_with_parent(path);

        /* When creating a new entry permissions are inherited from the parent node. */
        get_parent_perms(path, rec.e.perms);
    }

    /* Set the new value. */
    rec.e.value = val;

    /* Finally mark the entry as written and therefore as valid. */
    rec.e.write_seq = rec.next_seq++;

    return 0;
}

int lixs::mstore::simple_access::del(cid_t cid, const std::string& path)
{
    /* On deleting we can't create a new entry, so don't use the array operator. */
    database::iterator it;

    it = db.find(path);
    if (it == db.end()) {
        return ENOENT;
    }

    /* Get a reference for code clarity. */
    record& rec = it->second;

    /* If the entry is used in a transaction we might get here with an invalid entry, verify. */
    if (rec.e.write_seq > rec.e.delete_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            return EACCES;
        }

        /* Delete all children branches and unregister this entry.  */
        delete_branch(path, rec);
        unregister_from_parent(path);

        /* If the transaction list is empty, i.e. no transaction is currently referencing this
         * entry, we can remove it from the database. Otherwise just mark as deleted.
         */
        if (rec.te.empty()) {
            db.erase(it);
        } else {
            rec.e.delete_seq = rec.next_seq++;
        }

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::get_children(cid_t cid,
        const std::string& path, std::set<std::string>& resp)
{
    /* On reading we can't create a new entry, so don't use the array operator. */
    database::iterator it;

    it = db.find(path);
    if (it == db.end()) {
        return ENOENT;
    }

    /* Get a reference for code clarity. */
    record& rec = it->second;

    /* If the entry is used in a transaction we might get here with an invalid entry, verify. */
    if (rec.e.write_seq > rec.e.delete_seq) {
        if (!has_read_access(cid, rec.e.perms)) {
            return EACCES;
        }

        resp = rec.e.children;

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::simple_access::get_perms(cid_t cid,
        const std::string& path, permission_list& perms)
{
    /* On reading we can't create a new entry, so don't use the array operator. */
    database::iterator it;

    it = db.find(path);
    if (it == db.end()) {
        return ENOENT;
    }

    /* Get a reference for code clarity. */
    record& rec = it->second;

    /* If the entry is used in a transaction we might get here with an invalid entry, verify. */
    if (rec.e.write_seq > rec.e.delete_seq) {
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
    /* On setting permissions we can't create a new entry, so don't use the array operator. */
    database::iterator it;

    it = db.find(path);
    if (it == db.end()) {
        return ENOENT;
    }

    /* Get a reference for code clarity. */
    record& rec = it->second;

    /* If the entry is used in a transaction we might get here with an invalid entry, verify. */
    if (rec.e.write_seq > rec.e.delete_seq) {
        if (!has_write_access(cid, rec.e.perms)) {
            return EACCES;
        }

        rec.e.perms = perms;

        /* Writing sequence needs to be updated both for value and permissions. */
        rec.e.write_seq = rec.next_seq++;

        return 0;
    } else {
        return ENOENT;
    }
}

void lixs::mstore::simple_access::register_with_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    /* The root node can't be registered. */
    if (basename(path, parent, name)) {
        /* This method should only be called after ensuring the parent branch exists and is valid,
         * therefore we can just get the entry without checking for its validity.
         */
        record& rec = db[parent];

        rec.e.children.insert(name);

        rec.e.write_children_seq = rec.next_seq++;
    }
}

void lixs::mstore::simple_access::unregister_from_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    /* The root node isn't registered. */
    if (basename(path, parent, name)) {
        /* This method should only be called after ensuring the parent branch exists and is valid,
         * therefore we can just get the entry without checking for its validity.
         */
        record& rec = db[parent];

        rec.e.children.erase(name);

        rec.e.write_children_seq = rec.next_seq++;
    }
}

void lixs::mstore::simple_access::ensure_branch(cid_t cid, const std::string& path)
{
    bool created;
    std::string name;
    std::string parent;

    /* This method is indirectly recursing, break recursion if we get to the root node. */
    if (basename(path, parent, name)) {
        /* Method create won't perform any action in case the node exists already, therefore we
         * don't need to check before. It will also not recurse in that case so we don't need to
         * check for the result of the operation.
         */
        create(cid, parent, created);
    }
}

void lixs::mstore::simple_access::delete_branch(const std::string& path, record& rec)
{
    /* Method del will eventually call unregister_from_parent which will update rec.e.children,
     * deleting the element we're currently iterating on, therefore we don't use an iterator here.
     */
    while (!rec.e.children.empty()) {
        /* Deleting a subtree doesn't require access permissions. Only access to the root node, so
         * we delete as 0.
         */
        del(0, path + "/" + *(rec.e.children.begin()));
    }
}

void lixs::mstore::simple_access::get_parent_perms(const std::string& path, permission_list& perms)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        /* This method should only be called after ensuring the parent branch exists and is valid,
         * therefore we can just get the entry without checking for its validity.
         */
        perms = db[parent].e.perms;
    } else {
        /* Getting permissions for the root node yield the default of n0 */
        perms.clear();
        perms.push_back(permission(0, false, false));
    }
}

