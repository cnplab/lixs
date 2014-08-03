#include <lixs/domain.hh>

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


lixs::domainU::domainU(xenstore& xs, int domid, int port)
    : domain(xs, domid)
{
    interface = (xenstore_domain_interface*) xc_gnttab_map_grant_ref(xcg_handle, domid,
            GNTTAB_RESERVED_XENSTORE, PROT_READ|PROT_WRITE);

    remote_port = port;
    local_port = xc_evtchn_bind_interdomain(xce_handle, domid, remote_port);
}

lixs::domainU::~domainU()
{
    xc_gnttab_munmap(xcg_handle, interface, 1);
}

