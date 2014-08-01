#include <lixs/server.hh>
#include <lixs/xen_server.hh>

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
#include <xen/io/xs_wire.h>
}


const std::string lixs::xen_server::xsd_kva_path = "/proc/xen/xsd_kva";
const std::string lixs::xen_server::xsd_port_path = "/proc/xen/xsd_port";

lixs::xen_server::xen_server(iomux& io)
    : io(io)
{
    xc_handle = xc_interface_open(NULL, NULL, 0);
    xcg_handle = xc_gnttab_open(NULL, 0);
    xce_handle = xc_evtchn_open(NULL, 0);

    port = xenbus_evtchn();
    xc_evtchn_bind_interdomain(xce_handle, 0, port);
    interface = xenbus_map();
    xc_evtchn_notify(xce_handle, port);
    virq_port = xc_evtchn_bind_virq(xce_handle, VIRQ_DOM_EXC);

    io.add(*this, xc_evtchn_fd(xce_handle), fd_cb::fd_ev(true, false));
}

lixs::xen_server::~xen_server(void)
{
    xc_interface_close(xc_handle);
    xc_gnttab_close(xcg_handle);
}

void lixs::xen_server::handle(const fd_cb::fd_ev& events)
{
    char buff[1024];

    read(xc_evtchn_fd(xce_handle), buff, 1024);

    printf("lixs::xen_server::handle\n");
}

evtchn_port_t lixs::xen_server::xenbus_evtchn(void)
{
    evtchn_port_t port;

    std::ifstream file(xsd_port_path.c_str());
    file >> port;

    return port;
}

struct xenstore_domain_interface* lixs::xen_server::xenbus_map(void)
{
    int fd;
    struct xenstore_domain_interface* interface;

    fd = open(xsd_kva_path.c_str(), O_RDWR);
    interface = (xenstore_domain_interface*) mmap(
            NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    return interface;
}

