#ifndef __LIXS_XEN_SERVER_HH__
#define __LIXS_XEN_SERVER_HH__

#include <lixs/domain.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

#include <map>


namespace lixs {

class domain;

class xen_server {
public:
    xen_server(xenstore& xs, event_mgr& emgr);
    ~xen_server(void);

    void create_domain(int domid, int port);
    void destroy_domain(int domid);

private:
    xenstore& xs;
    event_mgr& emgr;

    std::map<int, lixs::domain*> domlist;
};

} /* namespace lixs */

#endif /* __LIXS_XEN_SERVER_HH__ */

