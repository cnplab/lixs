#include <lixs/mstore/database.hh>
#include <lixs/permissions.hh>

bool lixs::mstore::has_read_access(cid_t cid, const permission_list& perms)
{
    permission_list::const_iterator it;

    if (cid == 0
            || perms.size() == 0
            || perms.front().cid == cid
            || perms.front().read) {
        return true;
    }

    for (it = perms.begin(); it != perms.end(); it++) {
        if (it->cid == cid && it->read) {
            return true;
        }
    }

    return false;
}

bool lixs::mstore::has_write_access(cid_t cid, const permission_list& perms)
{
    permission_list::const_iterator it;

    if (cid == 0
            || perms.size() == 0
            || perms.front().cid == cid
            || perms.front().write) {
        return true;
    }

    for (it = perms.begin(); it != perms.end(); it++) {
        if (it->cid == cid && it->write) {
            return true;
        }
    }

    return false;
}

