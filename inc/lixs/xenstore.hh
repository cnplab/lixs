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

    int read(unsigned int tid, std::string path, std::string& res);
    int write(unsigned int tid, char* path, const char* val);
    int mkdir(unsigned int tid, char* path);
    int rm(unsigned int tid, char* path);
    int directory(unsigned int tid, const char* path, std::set<std::string>& res);

    int transaction_start(unsigned int* tid);
    int transaction_end(unsigned int tid, bool commit);

    void watch(watch_cb_k& cb);
    void unwatch(watch_cb_k& cb);

    void get_domain_path(int domid, char* buff);
    void get_domain_path(char* domid, char (&buff)[32]);
    void introduce_domain(int domid, int mfn, int port);
    void release_domain(int domid);
    void exists_domain(int domid, bool& exists);

private:
    store& st;

    event_mgr& emgr;
    watch_mgr wmgr;
    domain_mgr dmgr;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

