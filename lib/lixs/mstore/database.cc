#include <lixs/mstore/database.hh>
#include <lixs/permissions.hh>

bool lixs::mstore::has_read_access(cid_t cid, const permission_list& perms)
{
    if (cid == 0
            || perms.size() == 0
            || perms.front().cid == cid
            || perms.front().read) {
        return true;
    }

    for (auto& p : perms) {
        if (p.cid == cid && p.read) {
            return true;
        }
    }

    return false;
}

bool lixs::mstore::has_write_access(cid_t cid, const permission_list& perms)
{
    if (cid == 0
            || perms.size() == 0
            || perms.front().cid == cid
            || perms.front().write) {
        return true;
    }

    for (auto& p : perms) {
        if (p.cid == cid && p.write) {
            return true;
        }
    }

    return false;
}

