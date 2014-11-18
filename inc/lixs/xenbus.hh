#ifndef __LIXS_XENBUS_HH__
#define __LIXS_XENBUS_HH__

#include <lixs/domain.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

#include <string>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class xenbus_mapper {
public:
    xenbus_mapper(domid_t domid);
    ~xenbus_mapper();

    xenstore_domain_interface* get(void);

private:
    static const std::string xsd_kva_path;

    xenstore_domain_interface* interface;
};


class xenbus : public domain<xenbus_mapper> {
public:
    xenbus(xenstore& xs, event_mgr& emgr);
    ~xenbus();

private:
    static evtchn_port_t xenbus_evtchn(void);

    static const std::string xsd_port_path;
};

} /* namespace lixs */

#endif /* __LIXS_XENBUS_HH__ */

