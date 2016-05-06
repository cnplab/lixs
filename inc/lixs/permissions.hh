#ifndef __LIXS_PERMISSIONS_HH__
#define __LIXS_PERMISSIONS_HH__

#include <list>


namespace lixs {

typedef unsigned int cid_t;

struct permission {
public:
    permission(void)
        : cid(0), read(false), write(false)
    { }

    permission(cid_t cid)
        : cid(cid), read(false), write(false)
    { }

    permission(cid_t cid, bool read, bool write)
        : cid(cid), read(read), write(write)
    { }


    bool operator==(const permission& rhs) const {
        return (cid == rhs.cid) && (read == rhs.read) && (write == rhs.write);
    }


    cid_t cid;

    bool read;
    bool write;
};

typedef struct permission permission;
typedef std::list<permission> permission_list;

} /* namespace lixs */

#endif /* __LIXS_PERMISSIONS_HH__ */

