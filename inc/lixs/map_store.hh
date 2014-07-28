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
    const char* read(int id, std::string key);
    void write(int id, std::string key, std::string val);

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
            : val(val), r_time(0), w_time(0)
        { };

        const char* read(void)
        {
            r_time = get_time();
            return val.c_str();
        }

        void write(std::string new_val)
        {
            val = new_val;
            w_time = get_time();
        }

    private:
        std::string val;
        long int r_time;
        long int w_time;
    };

    std::map<std::string, record> data;
};

} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_HH__ */

