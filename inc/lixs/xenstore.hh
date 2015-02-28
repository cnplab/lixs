#ifndef __LIXS_XENSTORE_HH__
#define __LIXS_XENSTORE_HH__

#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/permissions.hh>
#include <lixs/store.hh>
#include <lixs/watch_mgr.hh>

#include <string>
#include <set>


namespace lixs {

class xenstore {
public:
    xenstore(store& st, event_mgr& emgr, iomux& io);
    ~xenstore();

    int store_read(cid_t cid, unsigned int tid,
            const std::string& path, std::string& val);
    int store_write(cid_t cid, unsigned int tid,
            const std::string& path, const std::string& val);
    int store_mkdir(cid_t cid, unsigned int tid,
            const std::string& path);
    int store_rm(cid_t cid, unsigned int tid,
            const std::string& path);
    int store_dir(cid_t cid, unsigned int tid,
            const std::string& path, std::set<std::string>& res);
    int store_get_perms(cid_t cid, unsigned int tid,
            const std::string& path, permission_list& resp);
    int store_set_perms(cid_t cid, unsigned int tid,
            const std::string& path, const permission_list& resp);

    int transaction_start(cid_t cid, unsigned int* tid);
    int transaction_end(cid_t cid, unsigned int tid, bool commit);

    void watch_add(watch_cb_k& cb);
    void watch_del(watch_cb_k& cb);

    /* FIXME: should domain operations also receive a client id? */
    void domain_path(domid_t domid, std::string& path);
    void domain_introduce(domid_t domid, unsigned int mfn, evtchn_port_t port);
    void domain_release(domid_t domid);
    void domain_exists(domid_t domid, bool& exists);

private:
    store& st;

    watch_mgr wmgr;
    domain_mgr dmgr;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

