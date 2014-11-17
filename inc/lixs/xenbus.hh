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

class xenbus : public domain {
public:
    xenbus(xenstore& xs, event_mgr& emgr);
    ~xenbus();

private:
    void map_ring(void);
    void unmap_ring(void);

    static evtchn_port_t xenbus_evtchn(void);

    static const std::string xsd_kva_path;
    static const std::string xsd_port_path;
};

} /* namespace lixs */

#endif /* __LIXS_XENBUS_HH__ */

