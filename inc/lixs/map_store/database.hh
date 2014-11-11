#ifndef __LIXS_MAP_STORE_DATABASE_HH__
#define __LIXS_MAP_STORE_DATABASE_HH__

#include <lixs/map_store/record.hh>

#include <map>
#include <string>


namespace lixs {
namespace map_store {

typedef std::map<std::string, record> database;

} /* namespace map_store */
} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_DATABASE_HH__ */

