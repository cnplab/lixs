#ifndef __LIXS_XENBUS_HH__
#define __LIXS_XENBUS_HH__

#include <lixs/client.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/ring_conn.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <string>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class xenbus_mapper {
protected:
    xenbus_mapper(domid_t domid);
    virtual ~xenbus_mapper();

protected:
    xenstore_domain_interface* interface;

private:
    static const std::string xsd_kva_path;
};


class xenbus : public client<ring_conn<xenbus_mapper> > {
public:
    xenbus(xenstore& xs, event_mgr& emgr, iomux& io);
    ~xenbus();

private:
    static evtchn_port_t xenbus_evtchn(void);

private:
    static const std::string xsd_port_path;
};

} /* namespace lixs */

#endif /* __LIXS_XENBUS_HH__ */

