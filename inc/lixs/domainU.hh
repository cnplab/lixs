#ifndef __LIXS_DOMAINU_HH__
#define __LIXS_DOMAINU_HH__

#include <lixs/domain.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

#include <sys/mman.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


namespace lixs {

class foreign_ring_mapper {
public:
    foreign_ring_mapper(domid_t domid);
    ~foreign_ring_mapper();

    xenstore_domain_interface* get(void);

private:
    xc_gnttab *xcg_handle;
    struct xenstore_domain_interface* interface;
};


class domainU : public domain<foreign_ring_mapper> {
public:
    domainU(xenstore& xs, event_mgr& emgr, domid_t domid, evtchn_port_t port);
    ~domainU();
};

} /* namespace lixs */

#endif /* __LIXS_DOMAINU_HH__ */

