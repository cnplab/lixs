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

private:
    class record {
    public:
        record(void)
            : val(""), r_time(get_time()), w_time(r_time)
        { };

        record(std::string val)
            : val(val), r_time(get_time()), w_time(r_time)
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
        long int get_time(void) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return (long int) tv.tv_sec * 1000 + tv.tv_usec / 1000;
        }

        std::string val;
        long int r_time;
        long int w_time;
    };

    std::map<std::string, record> data;
};

} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_HH__ */

