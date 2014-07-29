#ifndef __LIXS_MAP_STORE_HH__
#define __LIXS_MAP_STORE_HH__

#include <lixs/store.hh>

#include <map>
#include <string>
#include <sys/time.h>


namespace lixs {

class map_store : public store {
public:
    const char* read(std::string key);
    void write(std::string key, std::string val);
    void del(std::string key);

    void branch(int id);
    bool merge(int id);
    void abort(int id);
    const char* read(int id, std::string key);
    void write(int id, std::string key, std::string val);
    void del(int id, std::string key);

private:
    static long int get_time(void) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (long int) tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    class record {
    public:
        record(void)
            : val(""), r_time(0), w_time(0)
        { };

        record(std::string val)
            : val(val), r_time(0), w_time(get_time())
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

    class transaction {
    public:
        transaction(void)
            : time(get_time())
        { };

        long int time;
        std::map<std::string, record> data;
    };

    std::map<std::string, record> data;
    std::map<int, transaction> ltrans;
};

} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_HH__ */

