#ifndef __LIXS_XENSTORE_HH__
#define __LIXS_XENSTORE_HH__

#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/store.hh>
#include <lixs/watch_mgr.hh>

#include <string>
#include <set>


namespace lixs {

class xenstore {
public:
    xenstore(store& st, event_mgr& emgr);
    ~xenstore();

    int read(unsigned int tid, const std::string& path, std::string& val);
    int write(unsigned int tid, const std::string& path, const std::string& val);
    int mkdir(unsigned int tid, const std::string& path);
    int rm(unsigned int tid, const std::string& path);
    int directory(unsigned int tid, const std::string& path, std::set<std::string>& res);

    int transaction_start(unsigned int* tid);
    int transaction_end(unsigned int tid, bool commit);

    void watch(watch_cb_k& cb);
    void unwatch(watch_cb_k& cb);

    void get_domain_path(domid_t domid, std::string& path);
    void introduce_domain(domid_t domid, unsigned int mfn, evtchn_port_t port);
    void release_domain(domid_t domid);
    void exists_domain(domid_t domid, bool& exists);

private:
    store& st;

    event_mgr& emgr;
    watch_mgr wmgr;
    domain_mgr dmgr;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

