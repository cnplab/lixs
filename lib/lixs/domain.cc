#include <lixs/domain.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <cstddef>
#include <sys/mman.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


lixs::foreign_ring_mapper::foreign_ring_mapper(domid_t domid, unsigned int mfn)
{
    xcg_handle = xc_gnttab_open(NULL, 0);
    interface = (xenstore_domain_interface*) xc_gnttab_map_grant_ref(xcg_handle, domid,
            GNTTAB_RESERVED_XENSTORE, PROT_READ|PROT_WRITE);
}

lixs::foreign_ring_mapper::~foreign_ring_mapper()
{
    xc_gnttab_munmap(xcg_handle, interface, 1);
    xc_gnttab_close(xcg_handle);
}


lixs::domain::domain(xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io,
        domid_t domid, evtchn_port_t port, unsigned int mfn)
    : client(domid, get_id(domid), xs, dmgr, emgr, io, domid, port, mfn),
    active(true), domid(domid)
{
    std::string path;
    xs.domain_path(domid, path);

    std::sprintf(msg.abs_path, "%s/", path.c_str());
    msg.body = msg.abs_path + path.length() + 1;
}

lixs::domain::~domain()
{
}

bool lixs::domain::is_active(void)
{
    return active;
}

void lixs::domain::set_inactive(void)
{
    active = false;
}

domid_t lixs::domain::get_domid(void)
{
    return domid;
}

std::string lixs::domain::get_id(domid_t domid)
{
    return "D" + std::to_string(domid);
}

