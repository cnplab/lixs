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
    data.erase(key);
}

void lixs::map_store::branch(int id)
{
}

bool lixs::map_store::merge(int id)
{
    return false;
}

const char* lixs::map_store::read(int id, std::string key)
{
    return NULL;
}

void lixs::map_store::write(int id, std::string key, std::string val)
{
}

