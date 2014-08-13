#include <lixs/map_store.hh>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>


const char* lixs::map_store::read(std::string key)
{
    std::map<std::string, record>::iterator it;

    it = data.find(key);

    if (it == data.end()) {
        return NULL;
    } else {
        return it->second.read();
    }
}

void lixs::map_store::write(std::string key, std::string val)
{
    data[key].write(val);
}

void lixs::map_store::del(std::string key)
{
    std::map<std::string, record>::iterator it;

    it = data.find(key);

    if (it != data.end()) {
        it->second.erase();
    }
}

bool lixs::map_store::ensure(std::string key)
{
    std::map<std::string, record>::iterator it;

    it = data.find(key);

    if (it == data.end()) {
        data[key].write(std::string(""));
        return true;
    } else {
        return false;
    }
}

int lixs::map_store::get_childs(std::string key, const char* resp[], int nresp)
{
    int i;
    std::map<std::string, record>::iterator it;

    for (it = data.begin(), i = 0; it != data.end() && i < nresp; it++) {
        if (it->first.find(key) == 0 && it->first != key) {

            const char *r = it->first.c_str();

            r += key.length();
            if (*key.rbegin() != '/') {
                r++;
            }

            if (strchr(r, '/'))
                continue;

            resp[i++] = r;
        }
    }

    return i;
}

void lixs::map_store::branch(int id)
{
    ltrans[id];
}

bool lixs::map_store::merge(int id)
{
    bool abort = false;
    transaction& trans = ltrans[id];
    std::map<std::string, record>::iterator bd_rec;

    for (std::map<std::string, record>::iterator it = trans.data.begin();
            it != trans.data.end(); it++) {

        bd_rec = data.find(it->first);

        /* Transaction will abort if someone has write an entry that we read */
        if ((bd_rec != data.end()) &&
                ((it->second.r_time > 0) && (bd_rec->second.w_time >= trans.time))) {
            abort = true;
            break;
        }
    }

    if (!abort) {
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

    ltrans.erase(id);

    return !abort;
}

void lixs::map_store::abort(int id)
{
    ltrans.erase(id);
}

const char* lixs::map_store::read(int id, std::string key)
{
    if (id == 0) {
        return read(key);
    }

    std::map<std::string, record>::iterator it;
    transaction& trans = ltrans[id];

    it = trans.data.find(key);
    if (it != trans.data.end() && it->second.w_time > 0) {
        return it->second.read();
    }

    trans.data[key].read();

    it = data.find(key);
    if (it == data.end()) {
        return NULL;
    } else {
        return it->second.val.c_str();
    }
}

void lixs::map_store::write(int id, std::string key, std::string val)
{
    if (id == 0) {
        write(key, val);
        return;
    }

    ltrans[id].data[key].write(val);
}

void lixs::map_store::del(int id, std::string key)
{
    if (id == 0) {
        del(key);
        return;
    }

    ltrans[id].data[key].erase();
}

bool lixs::map_store::ensure(int id, std::string key)
{
    if (id == 0) {
        return ensure(key);
    }

    std::map<std::string, record>::iterator it;

    it = data.find(key);

    if (it == data.end()) {
        ltrans[id].data[key].write(std::string(""));
        return true;
    } else {
        return false;
    }
}
