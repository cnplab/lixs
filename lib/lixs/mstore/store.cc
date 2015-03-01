#include <lixs/mstore/store.hh>


lixs::mstore::store::store(void)
    : access(db), next_tid(1)
{
}

lixs::mstore::store::~store(void)
{
}

void lixs::mstore::store::branch(unsigned int& tid)
{
    tid = next_tid++;
    trans.insert({tid, transaction(tid, db)});
}

int lixs::mstore::store::merge(unsigned int tid, bool& success)
{
    transaction_db::iterator it;

    it = trans.find(tid);
    if (it != trans.end()) {
        it->second.merge(success);
        trans.erase(it);

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::store::abort(unsigned int tid)
{
    transaction_db::iterator it;

    it = trans.find(tid);
    if (it != trans.end()) {
        it->second.abort();
        trans.erase(it);

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::store::create(cid_t cid, unsigned int tid, std::string path, bool& created)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.create(cid, path, created);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.create(cid, path, created);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::read(cid_t cid, unsigned int tid, std::string path, std::string& val)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.read(cid, path, val);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.read(cid, path, val);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::update(cid_t cid, unsigned int tid, std::string path, std::string val)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.update(cid, path, val);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.update(cid, path, val);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::del(cid_t cid, unsigned int tid, std::string path)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.del(cid, path);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.del(cid, path);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::get_children(cid_t cid, unsigned int tid, std::string path,
        std::set<std::string>& resp)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.get_children(cid, path, resp);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.get_children(cid, path, resp);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::get_perms(cid_t cid, unsigned int tid,
        std::string path, permission_list& perms)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.get_perms(cid, path, perms);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.get_perms(cid, path, perms);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::set_perms(cid_t cid, unsigned int tid,
        std::string path, const permission_list& perms)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.set_perms(cid, path, perms);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.set_perms(cid, path, perms);
        } else {
            return EINVAL;
        }
    }
}

