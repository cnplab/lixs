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
    domain(xenstore& xs, event_mgr& emgr, int domid);
    virtual ~domain();

    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);
    void process_events(bool read, bool write);

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
    domainU(xenstore& xs, event_mgr& emgr, int domid, int port);
    ~domainU();
};

class domain0 : public domain {
public:
    domain0(xenstore&xs, event_mgr& emgr);
    ~domain0();

private:
    void map_ring(void);
    void unmap_ring(void);

    static evtchn_port_t xenbus_evtchn(void);

    static const std::string xsd_kva_path;
    static const std::string xsd_port_path;
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_HH__ */

