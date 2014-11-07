#include <lixs/map_store.hh>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <map>
#include <string>


lixs::map_store::map_store(void)
    : next_id(1)
{
}

lixs::map_store::~map_store()
{
}

void lixs::map_store::branch(unsigned int& id)
{
    id = next_id++;
    ltrans[id];
}

void lixs::map_store::merge(unsigned int id, bool& success)
{
    transaction& trans = ltrans[id];
    std::map<std::string, record>::iterator bd_rec;

    /* Determine if changes can be merged. */
    success = true;
    for (std::map<std::string, record>::iterator it = trans.data.begin();
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
        for (std::map<std::string, record>::iterator it = trans.data.begin();
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

void lixs::map_store::abort(unsigned int id)
{
    ltrans.erase(id);
}

int lixs::map_store::create(int id, std::string key, bool& created)
{
    if (key.back() == '/') {
        key.pop_back();
    }

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

int lixs::map_store::read(int id, std::string key, std::string& val)
{
    std::map<std::string, record>::iterator it;

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

int lixs::map_store::update(int id, std::string key, std::string val)
{
    if (key.back() == '/') {
        key.pop_back();
    }

    if (id == 0) {
        data[key].write(val);
    } else {
        ltrans[id].data[key].write(val);
    }

    return 0;
}

int lixs::map_store::del(int id, std::string key)
{
    std::map<std::string, record>::iterator it;

    if (key.back() == '/') {
        key.pop_back();
    }

    if (id == 0) {
        it = data.find(key);

        if (it != data.end()) {
            it->second.erase();
        }
    } else {
        ltrans[id].data[key].erase();
    }

    return 0;
}

int lixs::map_store::get_childs(std::string key, const char* resp[], int nresp)
{
    int i;
    std::map<std::string, record>::iterator it;

    if (key.back() == '/') {
        key.pop_back();
    }

    for (it = data.begin(), i = 0; it != data.end() && i < nresp; it++) {
        if (it->first.find(key) == 0) {
            const char *r = it->first.c_str() + key.length();

            if (*r != '/') {
                continue;
            }

            /* Remove the '/' */
            r++;

            /* We only want direct children */
            if (strchr(r, '/'))
                continue;

            resp[i++] = r;
        }
    }

    return i;
}

