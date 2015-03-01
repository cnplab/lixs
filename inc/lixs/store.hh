#ifndef __LIXS_STORE_HH__
#define __LIXS_STORE_HH__

#include <lixs/permissions.hh>

#include <set>
#include <string>


namespace lixs {

class store {
public:
    virtual void branch(unsigned int& tid) = 0;
    virtual int merge(unsigned int tid, bool& success) = 0;
    virtual int abort(unsigned int tid) = 0;

    virtual int create(cid_t cid, unsigned int tid,
            std::string path, bool& created) = 0;
    virtual int read(cid_t cid, unsigned int tid,
            std::string path, std::string& val) = 0;
    virtual int update(cid_t cid, unsigned int tid,
            std::string path, std::string val) = 0;
    virtual int del(cid_t cid, unsigned int tid,
            std::string path) = 0;

    virtual int get_children(cid_t cid, unsigned int tid,
            std::string path, std::set<std::string>& resp) = 0;

    virtual int get_perms(cid_t cid, unsigned int tid,
            std::string path, permission_list& perms) = 0;
    virtual int set_perms(cid_t cid, unsigned int tid,
            std::string path, const permission_list& perms) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_STORE_HH__ */

