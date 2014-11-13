#ifndef __LIXS_XENSTORE_HH__
#define __LIXS_XENSTORE_HH__

#include <lixs/events.hh>
#include <lixs/iomux.hh>
#include <lixs/store.hh>
#include <lixs/xen_server.hh>
#include <lixs/watch_mgr.hh>

#include <list>


namespace lixs {

class xen_server;

class xenstore {
public:
    xenstore(store& st, iomux& io);
    ~xenstore();

    void run(void);

    int read(unsigned int tid, std::string path, std::string& res);
    int write(unsigned int tid, char* path, const char* val);
    int mkdir(unsigned int tid, char* path);
    int rm(unsigned int tid, char* path);
    int directory(unsigned int tid, const char* path, std::list<std::string>& list);

    int transaction_start(unsigned int* tid);
    int transaction_end(unsigned int tid, bool commit);

    void watch(watch_cb_k& cb);
    void unwatch(watch_cb_k& cb);

    void get_domain_path(int domid, char* buff);
    void get_domain_path(char* domid, char (&buff)[32]);
    void introduce_domain(int domid, int mfn, int port);
    void release_domain(int domid);

    void once(ev_cb_k& cb);

    void add(fd_cb_k& cd);
    void set(fd_cb_k& cb);
    void remove(fd_cb_k& cb);

    void set_xen_server(xen_server* server);

private:
    void run_once_ev(void);

    store& st;

    watch_mgr wmgr;

    iomux& io;
    std::list<ev_cb_k*> once_lst;

    xen_server* xen;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

