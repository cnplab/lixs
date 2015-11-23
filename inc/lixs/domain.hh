#ifndef __LIXS_DOMAIN_HH__
#define __LIXS_DOMAIN_HH__

#include <lixs/client.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/ring_conn.hh>
#include <lixs/xenstore.hh>
#include <lixs/xs_proto_v1/xs_proto.hh>

#include <cerrno>
#include <sys/mman.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


namespace lixs {

class foreign_ring_mapper {
protected:
    foreign_ring_mapper(domid_t domid, unsigned int mfn);
    virtual ~foreign_ring_mapper();

protected:
    struct xenstore_domain_interface* interface;

private:
    xc_gnttab *xcg_handle;
};


class domain : public client<xs_proto_v1::xs_proto<ring_conn<foreign_ring_mapper> > > {
public:
    domain(ev_cb dead_cb, xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io,
            domid_t domid, evtchn_port_t port, unsigned int mfn);
    ~domain();

    bool is_active(void);
    void set_inactive(void);
    domid_t get_domid(void);

private:
    static std::string get_id(domid_t domid);

    void conn_dead(void);

private:
    event_mgr& emgr;
    ev_cb dead_cb;

    bool active;
    domid_t domid;
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_HH__ */

