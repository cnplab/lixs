#ifndef __LIXS_DOMAIN_MGR_HH__
#define __LIXS_DOMAIN_MGR_HH__

#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>

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
    typedef std::map<domid_t, domain*>::iterator iterator;

public:
    domain_mgr(xenstore& xs, event_mgr& emgr, iomux& io);
    ~domain_mgr();

    int create(domid_t domid, evtchn_port_t port, unsigned int mfn);
    int destroy(domid_t domid);
    void exists(domid_t domid, bool& exists);
    iterator begin(void);
    iterator end(void);

private:
    typedef std::map<domid_t, domain*> domain_map;

private:
    xenstore& xs;
    event_mgr& emgr;
    iomux& io;

    domain_map domains;
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_MGR_HH__ */

