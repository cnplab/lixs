#ifndef __LIXS_DOMAIN_HH__
#define __LIXS_DOMAIN_HH__

#include <lixs/client.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


namespace lixs {

class domain : public client {
public:
    domain(xenstore& xs, event_mgr& emgr, domid_t domid);
    virtual ~domain();

    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);
    void process_events(bool read, bool write);
    bool is_alive(void);

protected:
    xc_gnttab *xcg_handle;
    xc_evtchn *xce_handle;

    domid_t domid;
    evtchn_port_t local_port;
    evtchn_port_t remote_port;
    struct xenstore_domain_interface* interface;
};

class domainU : public domain {
public:
    domainU(xenstore& xs, event_mgr& emgr, domid_t domid, evtchn_port_t port);
    ~domainU();
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_HH__ */

