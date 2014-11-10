#ifndef __LIXS_MAP_STORE_TRANSACTION_HH__
#define __LIXS_MAP_STORE_TRANSACTION_HH__

#include <lixs/map_store/util.hh>

#include <map>
#include <string>


namespace lixs {
namespace map_store {

class transaction {
public:
    transaction(void)
        : time(get_time())
    { };

    long int time;
    std::map<std::string, record> data;
};

} /* namespace map_store */
} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_TRANSACTION_HH__ */

