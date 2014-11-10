#ifndef __LIXS_MAP_STORE_UTIL_HH__
#define __LIXS_MAP_STORE_UTIL_HH__



namespace lixs {
namespace map_store {


/* FIXME: I don't really like this solution!
 *
 * Maybe this can be moved to a time keeping class. An object will be
 * instantiated by map_store and a reference passed to records and
 * transactions.
 */
static long int get_time(void) {
    static long int curr_time = 1;
    return curr_time++;
}

} /* namespace map_store */
} /* namespace lixs */

#endif /* __LIXS_MAP_STORE_UTIL_HH__ */

