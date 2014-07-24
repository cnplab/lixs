#ifndef __LIXS_XEN_SERVER_HH__
#define __LIXS_XEN_SERVER_HH__

#include <lixs/server.hh>

/* Someone forgot this on xen headers */
#include <inttypes.h>
#include <string>

extern "C" {
#include <xenctrl.h>
#include <xen/io/xs_wire.h>
}


namespace lixs {

class xen_server : public server, public iokfd {
public:
    xen_server(iomux& io);
    ~xen_server(void);

    static const std::string xsd_kva_path;
    static const std::string xsd_port_path;

private:
    evtchn_port_t xenbus_evtchn(void);
    struct xenstore_domain_interface* xenbus_map(void);

    void handle(const ioev& events);

    iomux& io;

    xc_interface *xc_handle;
    xc_gnttab *xcg_handle;
    xc_evtchn *xce_handle;
    evtchn_port_t virq_port;

    /* Domain0 */
    evtchn_port_t port;
    struct xenstore_domain_interface* interface;
};

} /* namespace lixs */

#endif /* __LIXS_XEN_SERVER_HH__ */

