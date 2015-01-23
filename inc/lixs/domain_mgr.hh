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

    void create_domain(domid_t domid, evtchn_port_t port, unsigned int mfn);
    void destroy_domain(domid_t domid);
    void exists_domain(domid_t domid, bool& exists);

private:
    typedef std::map<domid_t, domain*> domain_map;


    xenstore& xs;
    event_mgr& emgr;

    domain_map domains;
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_MGR_HH__ */

