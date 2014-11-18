#include <lixs/map_store/store.hh>

#include <cstring>
#include <cerrno>
#include <list>
#include <string>


lixs::map_store::store::store(void)
    : next_id(1)
{
}

lixs::map_store::store::~store()
{
}

void lixs::map_store::store::branch(unsigned int& id)
{
    id = next_id++;
    ltrans[id];
}

void lixs::map_store::store::merge(unsigned int id, bool& success)
{
    transaction& trans = ltrans[id];
    database::iterator bd_rec;

    /* Determine if changes can be merged. */
    success = true;
    for (database::iterator it = trans.data.begin();
            it != trans.data.end(); it++) {

        bd_rec = data.find(it->first);

        /* Transaction will abort if someone has write an entry that we read */
        if ((bd_rec != data.end()) &&
                ((it->second.r_time > 0) && (bd_rec->second.w_time >= trans.time))) {
            success = false;
            break;
        }
    }

    /* Merge changes. */
    if (success) {
        for (database::iterator it = trans.data.begin();
                it != trans.data.end(); it++) {

            bd_rec = data.find(it->first);

            if (it->second.w_time > 0) {
                data[it->first].update_write(it->second);
                if (it->second.r_time > 0) {
                    data[it->first].update_read(it->second);
                }
            } else if (it->second.r_time > 0) {
                if (bd_rec != data.end()) {
                    bd_rec->second.update_read(it->second);
                }
            }
        }
    }

    /* Delete transaction information. */
    ltrans.erase(id);
}

void lixs::map_store::store::abort(unsigned int id)
{
    ltrans.erase(id);
}

int lixs::map_store::store::create(int id, std::string key, bool& created)
{
    if (key.back() == '/') {
        key.pop_back();
    }

    ensure_parents(id, key);

    if (data.find(key) == data.end()) {
        if (id == 0) {
            data[key].write(std::string(""));
        } else {
            ltrans[id].data[key].write(std::string(""));
        }

        created = true;
    } else {
        created = false;
    }

    return 0;
}

int lixs::map_store::store::read(int id, std::string key, std::string& val)
{
    database::iterator it;

    if (key.back() == '/') {
        key.pop_back();
    }

    if (id == 0) {
        it = data.find(key);

        if (it == data.end()) {
            return ENOENT;
        } else {
            it->second.read();
            val = it->second.val;
            return 0;
        }
    } else {
        transaction& trans = ltrans[id];

        it = trans.data.find(key);
        if (it != trans.data.end() && it->second.w_time > 0) {
            it->second.read();
            val = it->second.val;
            return 0;
        }

        trans.data[key].read();

        it = data.find(key);
        if (it == data.end()) {
            return ENOENT;
        } else {
            it->second.read();
            val = it->second.val;
            return 0;
        }
    }
}

int lixs::map_store::store::update(int id, std::string key, std::string val)
{
    if (key.back() == '/') {
        key.pop_back();
    }

    ensure_parents(id, key);

    if (id == 0) {
        data[key].write(val);
    } else {
        ltrans[id].data[key].write(val);
    }

    return 0;
}

int lixs::map_store::store::del(int id, std::string key)
{
    if (key.back() == '/') {
        key.pop_back();
    }

    delete_subtree(id, key);

    return 0;
}

void lixs::map_store::store::get_children(std::string key, std::list<std::string>& resp)
{
    database::iterator it;

    if (key.back() == '/') {
        key.pop_back();
    }

    for (it = data.begin(); it != data.end(); it++) {
        if (!it->second.deleted
                && it->first.find(key) == 0
                && it->first[key.length()] == '/'
                && it->first.find('/', key.length() + 1) == std::string::npos) {

            /* FIXME: handle transactions */

            resp.push_back(it->first.substr(key.length() + 1));
        }
    }
}

void lixs::map_store::store::ensure_parents(int id, const std::string& key)
{
    database::iterator it;
    std::string parent = key;
    size_t pos;

    for ( ; ; ) {
        pos = parent.rfind('/');
        if (pos == std::string::npos) {
            break;
        }

        parent = parent.substr(0, pos);

        if (id == 0) {
            it = data.find(parent);
            if (it != data.end() && !it->second.deleted) {
                break;
            }
        } else {
            it = ltrans[id].data.find(parent);
            if (it != ltrans[id].data.end() && !it->second.deleted) {
                break;
            }
        }

        if (id == 0) {
            data[parent].write("");
        } else {
            ltrans[id].data[parent].write("");
        }
    }
}

void lixs::map_store::store::delete_subtree(int id, const std::string& key)
{
    database::iterator it;

    it = data.find(key);
    if (it != data.end()) {
        /* Delete root */
        if (id == 0) {
            it->second.erase();
        } else {
            ltrans[id].data[key].erase();
        }

        /* Delete children */
        for (it = data.begin(); it != data.end(); it++) {
            if (!it->second.deleted
                    && it->first.find(key) == 0
                    && it->first[key.length()] == '/') {
                if (id == 0) {
                    it->second.erase();
                } else {
                    ltrans[id].data[it->first].erase();
                }
            }
        }
    }

    if (id != 0) {
        it = ltrans[id].data.find(key);
        if (it != ltrans[id].data.end()) {
            it->second.erase();
        }

        for (it = ltrans[id].data.begin(); it != ltrans[id].data.end(); it++) {
            if (!it->second.deleted
                    && it->first.find(key) == 0
                    && it->first[key.length()] == '/') {
                it->second.erase();
            }
        }
    }
}
