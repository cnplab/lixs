#include <lixs/xenbus.hh>

#include <cerrno>
#include <cstdlib>
#include <cstring>
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


const std::string lixs::xenbus_mapper::xsd_kva_path = "/proc/xen/xsd_kva";
const std::string lixs::xenbus::xsd_port_path = "/proc/xen/xsd_port";

lixs::xenbus_mapper::xenbus_mapper(domid_t domid)
{
    int fd;

    fd = open(xsd_kva_path.c_str(), O_RDWR);
    interface = (xenstore_domain_interface*) mmap(
            NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
}

lixs::xenbus_mapper::~xenbus_mapper(void)
{
    munmap(interface, getpagesize());
    interface = NULL;
}

xenstore_domain_interface* lixs::xenbus_mapper::get(void)
{
    return interface;
}


lixs::xenbus::xenbus(xenstore& xs, event_mgr& emgr)
    : domain(xs, emgr, 0)
{
    remote_port = xenbus_evtchn();
    local_port = xc_evtchn_bind_interdomain(xce_handle, 0, remote_port);
    xc_evtchn_notify(xce_handle, local_port);

    fd_cb.ev_read = true;
    emgr.io_set(fd_cb);
}

lixs::xenbus::~xenbus()
{
}

evtchn_port_t lixs::xenbus::xenbus_evtchn(void)
{
    evtchn_port_t port;

    std::ifstream file(xsd_port_path.c_str());
    file >> port;

    return port;
}

