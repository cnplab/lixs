#include <lixs/log/logger.hh>
#include <lixs/mstore/database.hh>
#include <lixs/mstore/transaction.hh>
#include <lixs/util.hh>

#include <set>
#include <string>


lixs::mstore::transaction::transaction(unsigned int id, database& db, log::logger& log)
    : db_access(db, log), id(id)
{
}

int lixs::mstore::transaction::create(cid_t cid, const std::string& path, bool& created)
{
    record& rec = db[path];
    tentry& te = get_tentry(path, rec);

    if (te.write_seq > te.delete_seq) {
        created = false;
    } else {
        /* Ensure the parent branch exists and is valid and register the new entry. */
        ensure_branch(cid, path);
        register_with_parent(path);

        /* Set data to the defaults: empty value and the permissions inherited from parent. */
        /* If the entry was deleted before during this transaction we can get here with a non-empty
         * te.value, so make sure to reset it.
         */
        te.value = "";
        get_parent_perms(path, te.perms);

        /* Finally mark the entry as written and therefore as valid. */
        te.write_seq = rec.next_seq++;

        created = true;
    }

    return 0;
}

int lixs::mstore::transaction::read(cid_t cid, const std::string& path, std::string& val)
{
    record& rec = db[path];
    tentry& te = get_tentry(path, rec);

    if (te.write_seq > te.delete_seq) {
        /* The entry is valid so fetch its data marking it as read. */
        fetch_tentry_data(te, rec);

        if (!has_read_access(cid, te.perms)) {
            return EACCES;
        }

        val = te.value;

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::transaction::update(cid_t cid, const std::string& path, const std::string& val)
{
    record& rec = db[path];
    tentry& te = get_tentry(path, rec);

    if (te.write_seq > te.delete_seq) {
        /* Although we're writing, if the entry already exists we need to fetch data, i.e. we need
         * to read it, to check permissions before actually writing to it. This will effectively
         * mark the entry as read.
         */
        fetch_tentry_data(te, rec);

        if (!has_write_access(cid, te.perms)) {
            return EACCES;
        }
    } else {
        /* Creating a new entry here: ensure the parent branch exists and is valid and register the
         * new entry.
         */
        ensure_branch(cid, path);
        register_with_parent(path);

        /* When creating a new entry permissions are inherited from the parent node. */
        get_parent_perms(path, te.perms);
    }

    /* Set the new value. */
    te.value = val;

    /* Finally mark the entry as written and therefore as valid. */
    te.write_seq = rec.next_seq++;

    return 0;
}

int lixs::mstore::transaction::del(cid_t cid, const std::string& path)
{
    record& rec = db[path];
    tentry& te = get_tentry(path, rec);

    if (te.write_seq > te.delete_seq) {
        /* Before deleting we need to fetch data, i.e. to read the entry, to check permissions. */
        fetch_tentry_data(te, rec);

        if (!has_write_access(cid, te.perms)) {
            return EACCES;
        }
    } else {
        return ENOENT;
    }

    /* If we have a valid entry and can write to it we then need to fetch the children list. */
    fetch_tentry_children(te, rec);

    /* Delete all children branches and unregister this entry.  */
    delete_branch(path, te);
    unregister_from_parent(path);

    /* We don't need to reset value or permissions. If the entry is re-used we reset the data
     * during creation.
     */

    /* Finally mark the entry as deleted and therefore as invalid. */
    te.delete_seq = rec.next_seq++;

    return 0;
}

int lixs::mstore::transaction::get_children(cid_t cid, const std::string& path, std::set<std::string>& resp)
{
    record& rec = db[path];
    tentry& te = get_tentry(path, rec);

    if (te.write_seq > te.delete_seq) {
        /* Entry data needs to be fetched to check for permissions. */
        fetch_tentry_data(te, rec);

        if (!has_read_access(cid, te.perms)) {
            return EACCES;
        }
    } else {
        return ENOENT;
    }

    /* If we have a valid entry and can read from it we then need to fetch the children list. */
    fetch_tentry_children(te, rec);

    /* Build the current children list as: `te.children + te.children_add - te.children_rem`. */
    resp = te.children;
    for (auto& c : te.children_add) {
        resp.insert(c);
    }
    for (auto& c : te.children_rem) {
        resp.erase(c);
    }

    return 0;
}

int lixs::mstore::transaction::get_perms(cid_t cid,
        const std::string& path, permission_list& perms)
{
    record& rec = db[path];
    tentry& te = get_tentry(path, rec);

    if (te.write_seq > te.delete_seq) {
        /* Fetch entry data here before checking reading permissions. */
        fetch_tentry_data(te, rec);

        if (!has_read_access(cid, te.perms)) {
            return EACCES;
        }
    } else {
        return ENOENT;
    }

    perms = te.perms;

    return 0;
}

int lixs::mstore::transaction::set_perms(cid_t cid,
        const std::string& path, const permission_list& perms)
{
    record& rec = db[path];
    tentry& te = get_tentry(path, rec);

    if (te.write_seq > te.delete_seq) {
        /* Although we're writing permissions we need to fetch entry data to
         * check permissions before.
         */
        fetch_tentry_data(te, rec);

        if (!has_write_access(cid, te.perms)) {
            return EACCES;
        }
    } else {
        return ENOENT;
    }

    te.perms = perms;

    /* Finally update writing time. */
    te.write_seq = rec.next_seq++;

    return 0;
}

void lixs::mstore::transaction::abort()
{
    for (auto& r : records) {
        record& rec = db[r];

        /* Remove transaction information from the entry. */
        rec.te.erase(id);

        /* If the entry is invalid and has no more active transactions we clean it from the DB. */
        if ((rec.e.delete_seq > rec.e.write_seq) && rec.te.empty()) {
            db.erase(r);
        }
    }

    /* Finally clear the record database. The transaction shouldn't be re-used but just in case. */
    records.clear();
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
    log::LOG<log::level::TRACE>::logf(log, "mstore::transaction::can_merge %d", id);

    /* The transaction should only succeed if, for each of the records referenced during the
     * transaction, three conditions are meet:
     */
    log::LOG<log::level::TRACE>::logf(log, "  RECORDS");
    for (auto& r : records) {
        record& rec = db[r];
        tentry& te = rec.te[id];

        /* 1. A valid entry at initialization time was deleted or an invalid entry at
         * initialization time was, created outside transaction;
         */
        if (te.init_valid) {
            if (rec.e.delete_seq > te.init_seq) {
                log::LOG<log::level::TRACE>::logf(log,
                        "    '%s' ABORT (rec.e.delete_seq > te.init_seq)", r.c_str());
                return false;
            }
        } else {
            if (rec.e.write_seq > te.init_seq) {
                log::LOG<log::level::TRACE>::logf(log,
                        "    '%s' ABORT (rec.e.write_seq > te.init_seq)", r.c_str());
                return false;
            }
        }

        /* 2. The entry was written outside of the transaction after the first time it was read
         * inside the transaction;
         */
        if (te.read_seq && rec.e.write_seq > te.read_seq) {
            log::LOG<log::level::TRACE>::logf(log,
                    "    '%s' ABORT (te.read_seq && rec.e.write_seq > te.read_seq)", r.c_str());
            return false;
        }

        /* 3. The children list of an entry changed outside of the transaction after the first time
         * it was read inside the transaction.
         */
        if (te.read_children_seq && rec.e.write_children_seq > te.read_children_seq) {
            log::LOG<log::level::TRACE>::logf(log,
                    "    '%s' ABORT "
                    "(te.read_children_seq && rec.e.write_children_seq > te.read_children_seq)",
                    r.c_str());
            return false;
        }

        log::LOG<log::level::TRACE>::logf(log, "    '%s' MERGE", r.c_str());
    }

    return true;
}

void lixs::mstore::transaction::do_merge()
{
    for (auto& r : records) {
        record& rec = db[r];
        tentry& te = rec.te[id];

        if (te.write_seq > te.delete_seq) {
            /* We should only update entry's data if the entry was written during transaction, i.e.
             * after initialization.
             */
            if (te.write_seq > te.init_seq) {
                rec.e.value = te.value;
                rec.e.perms = te.perms;

                /* If a second transaction started after this entry was written here, that
                 * transaction would be able to commit even though this transaction is modifying
                 * the entry now. Therefore we need to update the sequence numbers with new ones.
                 */
                rec.e.write_seq = rec.next_seq++;
            }

            /* If there were changes to the children list during transaction we apply those now and
             * update the write_children_seq sequence number.
             */
            if (!te.children_add.empty() || !te.children_rem.empty()) {
                for (auto& c : te.children_add) {
                    rec.e.children.insert(c);
                }
                for (auto& c : te.children_rem) {
                    rec.e.children.erase(c);
                }

                rec.e.write_children_seq = rec.next_seq++;
            }

            /* It is possible to get here without applying any action on the node, for instance,
             * when the node was simply read during the transaction.
             */
        } else {
            if (te.delete_seq > te.init_seq) {
                /* See above for why we need to update the sequence number. */
                te.delete_seq = rec.next_seq++;
            }
        }

        /* Delete the transaction information from the entry. */
        rec.te.erase(id);

        /* If no more transactions reference this entry we might be able to do some cleanup. */
        if (rec.te.empty()) {
            if (rec.e.write_seq > rec.e.delete_seq) {
                /* If the entry is valid we can reset sequence numbers to avoid overflows. */
                rec.next_seq = 2;
                rec.e.write_seq = 1;
                rec.e.delete_seq = 0;
                rec.e.write_children_seq = 0;
            } else {
                /* If the entry is invalid we should remove it from the database. */
                db.erase(r);
            }
        }
    }

    /* Finally clear the record database. The transaction shouldn't be re-used but just in case. */
    records.clear();
}

void lixs::mstore::transaction::register_with_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    /* The root node can't be registered. */
    if (basename(path, parent, name)) {
        record& rec = db[parent];
        tentry& te = get_tentry(parent, rec);

        te.children_add.insert(name);
        te.children_rem.erase(name);
    }
}

void lixs::mstore::transaction::unregister_from_parent(const std::string& path)
{
    std::string name;
    std::string parent;

    /* Check whether we're registering the root node, that is not registered anywhere. */
    if (basename(path, parent, name)) {
        record& rec = db[parent];
        tentry& te = get_tentry(parent, rec);

        te.children_rem.insert(name);
        te.children_add.erase(name);
    }
}

void lixs::mstore::transaction::ensure_branch(cid_t cid, const std::string& path)
{
    bool created;
    std::string name;
    std::string parent;

    /* This method is indirectly recursing, break recursion if we get to the root node. */
    if (basename(path, parent, name)) {
        /* Method create won't perform any action in case the node exists already, therefore we
         * don't need to check before calling. It will also not recurse in that case so we also
         * don't need to check for the result of the operation.
         */
        create(cid, parent, created);
    }
}

void lixs::mstore::transaction::delete_branch(const std::string& path, tentry& te)
{
    /* With due precaution we could operate on te.children directly. However it's safer to generate
     * the current children list on a separate set without touching te.children. Later we can
     * optimize this if necessary.
     */
    std::set<std::string> children;

    /* Build the current children list as: `te.children + te.children_add - te.children_rem`. */
    children = te.children;
    for (auto& c : te.children_add) {
        children.insert(c);
    }
    for (auto& c : te.children_rem) {
        children.erase(c);
    }

    /* Delete all children. This will recursively delete the full branches. */
    for (auto& c : children) {
        /* Deleting a subtree only requires write access to the root node, so delete as 0. */
        del(0, path + "/" + c);
    }

    /* During the deletion process children_rem will be updated with all the deleted children so
     * that `te.children + te.children_add - te.children_rem` results in an empty set. Therefore we
     * don't need to clear the sets. Again this can be optimized later if necessary.
     */
}

void lixs::mstore::transaction::get_parent_perms(const std::string& path, permission_list& perms)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        /* This method should only be called after ensuring the parent branch exists and is valid,
         * therefore we can just get the entry without checking for its validity. However we need
         * to make sure to fetch entry data so that we can read its permission list.
         */
        record& rec = db[parent];
        tentry& te = get_tentry(parent, rec);
        fetch_tentry_data(te, rec);

        perms = te.perms;
    } else {
        /* Getting permissions for the root node yield the default of n0 */
        perms.clear();
        perms.push_back(permission(0, false, false));
    }
}

lixs::mstore::tentry& lixs::mstore::transaction::get_tentry(const std::string& path, record& rec)
{
    tentry_map::iterator it;

    /* Transaction entries should *always* be obtained through this method. Therefore if the entry
     * already exists in the database, it was initialized already and we can just return it.
     */
    it = rec.te.find(id);
    if (it == rec.te.end()) {
        /* The first time the entry is referenced in the transaction we create it and add the path
         * to the records list.
         */
        records.insert(path);
        it = rec.te.insert(it, {id, {}});

        /* Get a reference just for code clarity. */
        tentry& te = it->second;

        /* The most basic information we need about an entry is its validity, i.e. if it exists or
         * not. For that we fetch write_seq and delete_seq. We don't fetch any other information as
         * that information might not be necessary, depending on the operation. For that we should
         * use fetch_tentry_data and fetch_tentry_children. Fetching entry data here would require
         * to mark the node as read, that would eventually cause to unnecessary aborts if the data
         * is actually never read.
         */
        te.write_seq = rec.e.write_seq;
        te.delete_seq = rec.e.delete_seq;

        /* Given write_seq and delete_seq are going to be used during the transaction (in case the
         * entry is written to), we also store on init_valid the initial state of the entry after
         * it was first referenced. The transaction should abort if a valid entry was deleted, or
         * an invalid entry was created, after the transaction start.
         */
        te.init_valid = te.write_seq > te.delete_seq;

        /* Finally we need to record the initialization sequence. This is used to check whether the
         * entry was written to after the transaction started.
         */
        te.init_seq = rec.next_seq++;
    }

    return it->second;
}

void lixs::mstore::transaction::fetch_tentry_data(tentry& te, record& rec)
{
    /* Entry data is fetch when the entry needs to be read from for the first time. We don't care
     * about subsequent reads. The transaction should abort if the entry was modified (written to)
     * outside of this transaction, after we read it for the first time during transaction.
     */
    if (!te.read_seq) {
        /* We only fetch information if the entry was not already written to during transaction. */
        if (te.write_seq < te.init_seq && te.delete_seq < te.init_seq) {
            /* It is possible we get here after the entry was deleted outside of the transaction.
             * For instance in the following scenario:
             *
             * 1. A call to create initializes the entry inside the transaction. The entry is valid
             * so no more actions are performed;
             * 2. The entry is deleted outside of the transaction;
             * 3. A call to read verifies that the entry should exist. This method is then called
             * to fetch the information from the underlying entry, which was already deleted.
             *
             * Given the entry was deleted the data is theoretically invalid, however in this
             * situation the transaction will abort given a valid entry was deleted outside of the
             * transaction after initialized here, and therefore we don't care too much about the
             * validity of the data we're fetching, the data is also not deleted from the entry
             * when it is marked as deleted, therefore we use it anyway.
             */
            te.value = rec.e.value;
            te.perms = rec.e.perms;
        }

        /* Even when the data isn't fetch we need to mark the entry as read. */
        te.read_seq = rec.next_seq++;
    }
}

void lixs::mstore::transaction::fetch_tentry_children(tentry& te, record& rec)
{
    /* Changing the children list of a node shouldn't make a transaction abort even if the node is
     * written to during the transaction. Therefore we should only fetch children metadata if we're
     * operating on the tree, i.e. for a get_children or a del. Of course we should only fetch
     * children information the first time it is necessary. A transaction should abort if the
     * children list changes outside of transaction after it was read during the transaction.
     */
    if (!te.read_children_seq) {
        /* Similarly to what happens on fetch_tentry_data we can get here after the underlying
         * entry was deleted. However in this case we simply let the children list blank: first it
         * doesn't matter a lot because the transaction will abort anyway; second if we actually
         * give a children list, that list will probably point to entries that already don't exist
         * on the database or in the transaction which cannot happen.
         */
        if (rec.e.write_seq > rec.e.delete_seq) {
            te.children.insert(rec.e.children.begin(), rec.e.children.end());
        }

        /* Finally mark the children list as fetched. */
        te.read_children_seq = rec.next_seq++;
    }
}

