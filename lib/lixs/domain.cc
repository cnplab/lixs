#include <lixs/domain.hh>
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


lixs::domain::domain(xenstore& xs, event_mgr& emgr,
        domid_t domid, evtchn_port_t port, unsigned int mfn)
    : client(xs, emgr, domid, port, mfn)
{
#ifdef DEBUG
    asprintf(&cid, "D%d", domid);
    printf("%4s = new conn\n", cid);
#endif

    xs.get_domain_path(domid, abs_path);
    body = abs_path + strlen(abs_path);
    *body++ = '/';
}

lixs::domain::~domain()
{
#ifdef DEBUG
    printf("%4s = closed conn\n", cid);
    free(cid);
#endif
}

