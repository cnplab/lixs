#ifndef __LIXS_XENBUS_HH__
#define __LIXS_XENBUS_HH__

#include <lixs/client.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/log/logger.hh>
#include <lixs/ring_conn.hh>
#include <lixs/xenstore.hh>
#include <lixs/xs_proto_v1/xs_proto.hh>

#include <cerrno>
#include <stdexcept>
#include <string>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class xenbus_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class xenbus_mapper {
protected:
    xenbus_mapper(domid_t domid);
    virtual ~xenbus_mapper();

protected:
    xenstore_domain_interface* interface;

private:
    static const std::string xsd_kva_path;
};


class xenbus : public client<xs_proto_v1::xs_proto<ring_conn<xenbus_mapper> > > {
public:
    xenbus(xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io, log::logger& log);
    ~xenbus();

private:
    static evtchn_port_t xenbus_evtchn(void);

    void conn_dead(void);

private:
    static const std::string xsd_port_path;
};

} /* namespace lixs */

#endif /* __LIXS_XENBUS_HH__ */

