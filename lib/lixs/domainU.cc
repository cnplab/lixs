#include <lixs/domainU.hh>

#include <cerrno>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


lixs::foreign_ring_mapper::foreign_ring_mapper(domid_t domid)
{
    xcg_handle = xc_gnttab_open(NULL, 0);

    interface = (xenstore_domain_interface*) xc_gnttab_map_grant_ref(xcg_handle, domid,
            GNTTAB_RESERVED_XENSTORE, PROT_READ|PROT_WRITE);
}

lixs::foreign_ring_mapper::~foreign_ring_mapper()
{
    xc_gnttab_munmap(xcg_handle, interface, 1);
    interface = NULL;

    xc_gnttab_close(xcg_handle);
    xcg_handle = NULL;
}

xenstore_domain_interface* lixs::foreign_ring_mapper::get(void)
{
    return interface;
}

lixs::domainU::domainU(xenstore& xs, event_mgr& emgr, domid_t domid, evtchn_port_t port)
    : domain(xs, emgr, domid)
{
    remote_port = port;
    local_port = xc_evtchn_bind_interdomain(xce_handle, domid, remote_port);

    xc_evtchn_unmask(xce_handle, local_port);
}

lixs::domainU::~domainU()
{
}

