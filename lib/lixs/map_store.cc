#include <lixs/map_store.hh>

#include <cstddef>
#include <cstdio>
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

        if (bd_rec != data.end() &&
                (bd_rec->second.r_time >= trans.time || bd_rec->second.w_time >= trans.time)) {
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
    const char* ret = read(key);

    ltrans[id].data[key].read();

    return ret;
}

void lixs::map_store::write(int id, std::string key, std::string val)
{
    ltrans[id].data[key].write(val);
}

void lixs::map_store::del(int id, std::string key)
{
    ltrans[id].data[key].erase();
}

