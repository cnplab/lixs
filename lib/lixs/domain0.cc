#include <lixs/domain.hh>

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


const std::string lixs::domain0::xsd_kva_path = "/proc/xen/xsd_kva";
const std::string lixs::domain0::xsd_port_path = "/proc/xen/xsd_port";

lixs::domain0::domain0(xenstore& xs)
    : domain(xs, 0)
{
    map_ring();

    remote_port = xenbus_evtchn();
    local_port = xc_evtchn_bind_interdomain(xce_handle, 0, remote_port);
    xc_evtchn_notify(xce_handle, local_port);

    virq_port = xc_evtchn_bind_virq(xce_handle, VIRQ_DOM_EXC);

    fd_cb.ev_read = true;
    xs.set(fd_cb);
}

lixs::domain0::~domain0()
{
    unmap_ring();
}

void lixs::domain0::map_ring(void)
{
    int fd;

    fd = open(xsd_kva_path.c_str(), O_RDWR);
    interface = (xenstore_domain_interface*) mmap(
            NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
}

void lixs::domain0::unmap_ring(void)
{
    munmap(interface, getpagesize());
}

evtchn_port_t lixs::domain0::xenbus_evtchn(void)
{
    evtchn_port_t port;

    std::ifstream file(xsd_port_path.c_str());
    file >> port;

    return port;
}

void lixs::domain0::process_events(bool read, bool write)
{
    evtchn_port_t port = xc_evtchn_pending(xce_handle);
    xc_evtchn_unmask(xce_handle, port);

    if (port == virq_port) {
        /* TODO: implement domain cleanup */
    }
}

