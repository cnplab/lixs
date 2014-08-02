#ifndef __LIXS_XENSTORE_HH__
#define __LIXS_XENSTORE_HH__

#include <lixs/events.hh>
#include <lixs/iomux.hh>
#include <lixs/store.hh>
#include <lixs/unix_server.hh>
#include <lixs/xen_server.hh>

#include <list>


namespace lixs {

class unix_server;
class xen_server;

class xenstore {
public:
    xenstore(store& st, iomux& io);
    ~xenstore();

    void run(void);

    int read(unsigned int tid, const char* path, const char** res);
    int write(unsigned int tid, char* path, const char* val);
    int mkdir(unsigned int tid, const char* path);
    int rm(unsigned int tid, const char* path);
    int directory(unsigned int tid, const char* path, const char* list[], int* nelems);

    int transaction_start(unsigned int* tid);
    int transaction_end(unsigned int tid, bool commit);

    void get_domain_path(char* domid, char (&buff)[32]);

    void once(ev_cb_k& cb);

    void add(fd_cb_k& cd);
    void set(fd_cb_k& cb);
    void remove(fd_cb_k& cb);

    void set_xen_server(xen_server* server);
    void set_unix_server(unix_server* server);

private:
    unsigned int next_tid;

    store& st;

    iomux& io;
    std::list<ev_cb_k*> once_lst;

    unix_server* nix;
    xen_server* xen;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

