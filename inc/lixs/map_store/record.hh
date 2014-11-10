#ifndef __LIXS_MAP_STORE_RECORD_HH__
#define __LIXS_MAP_STORE_RECORD_HH__

#include <lixs/map_store/util.hh>

#include <string>


namespace lixs {
namespace map_store {

class record {
public:
    record(void)
        : val(""), deleted(false), r_time(0), w_time(0)
    { };

    record(std::string val)
        : val(val), deleted(false), r_time(0), w_time(get_time())
    { };

    const char* read(void)
    {
        if (deleted) {
            return NULL;
        } else {
            r_time = get_time();
            return val.c_str();
        }
    }

    void write(std::string new_val)
    {
        w_time = get_time();
        val = new_val;
        deleted = false;
    }

    void erase(void)
    {
        w_time = get_time();
        deleted = true;
    }

    bool is_deleted(void)
    {
        return deleted;
    }

    void update_write(const record& r)
    {
        val = r.val;
        w_time = r.w_time;
        deleted = r.deleted;
    }

    void update_read(const record& r)
    {
        r_time = r.r_time;
    }

    std::string val;
    bool deleted;
    long int r_time;
    long int w_time;
};

} /* namespace map_store */
} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_RECORD_HH__ */

