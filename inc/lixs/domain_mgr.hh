#ifndef __LIXS_DOMAIN_MGR_HH__
#define __LIXS_DOMAIN_MGR_HH__

#include <lixs/event_mgr.hh>

#include <cerrno>
#include <map>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class domain;
class xenstore;

class domain_mgr {
public:
    domain_mgr(xenstore& xs, event_mgr& emgr);
    ~domain_mgr();

    void create_domain(domid_t domid, evtchn_port_t port);
    void destroy_domain(domid_t domid);

private:
    xenstore& xs;
    event_mgr& emgr;

    std::map<domid_t, lixs::domain*> domlist;
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_MGR_HH__ */

