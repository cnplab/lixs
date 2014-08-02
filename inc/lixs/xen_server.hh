#ifndef __LIXS_XEN_SERVER_HH__
#define __LIXS_XEN_SERVER_HH__

#include <lixs/events.hh>
#include <lixs/server.hh>
#include <lixs/xenstore.hh>

/* Someone forgot this on xen headers */
#include <inttypes.h>
#include <string>
#include <errno.h>

extern "C" {
#include <xenctrl.h>
#include <xen/io/xs_wire.h>
}


namespace lixs {

class xenstore;

class xen_server : public server {
public:
    xen_server(xenstore& xs);
    ~xen_server(void);

    static const std::string xsd_kva_path;
    static const std::string xsd_port_path;

private:
    class fd_cb_k : public lixs::fd_cb_k {
    public:
        fd_cb_k(xen_server& server)
            : server(server)
        { };

        void operator() (bool ev_read, bool ev_write);

        xen_server& server;
    };

    evtchn_port_t xenbus_evtchn(void);
    struct xenstore_domain_interface* xenbus_map(void);

    xenstore& xs;
    fd_cb_k fd_cb;

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

