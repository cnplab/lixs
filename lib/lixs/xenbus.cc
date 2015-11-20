#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenbus.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


const std::string lixs::xenbus_mapper::xsd_kva_path = "/proc/xen/xsd_kva";

lixs::xenbus_mapper::xenbus_mapper(domid_t domid)
{
    int fd;
    void* ptr;

    fd = open(xsd_kva_path.c_str(), O_RDWR);
    if (fd == -1) {
        throw xenbus_error("Unable to open " + xsd_kva_path + ": " +
                std::string(std::strerror(errno)));
    }

    ptr = mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (interface == MAP_FAILED) {
        close(fd);
        throw xenbus_error("Cannot mmap interface: " +
                std::string(std::strerror(errno)));
    }

    close(fd);

    interface = static_cast<xenstore_domain_interface*>(ptr);
}

lixs::xenbus_mapper::~xenbus_mapper(void)
{
    munmap(interface, getpagesize());
    interface = NULL;
}


const std::string lixs::xenbus::xsd_port_path = "/proc/xen/xsd_port";

/* FIXME: What is the correct domid when running in a stub domain? */
lixs::xenbus::xenbus(xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io)
    : client("XB", 0, xs, dmgr, io, 0, xenbus_evtchn())
{
}

lixs::xenbus::~xenbus()
{
}

evtchn_port_t lixs::xenbus::xenbus_evtchn(void)
{
    evtchn_port_t port;

    try {
        std::ifstream file(xsd_port_path.c_str());

        if (!file) {
            throw xenbus_error("Cannot read port from " + xsd_port_path);
        }

        file >> port;
    } catch (std::ifstream::failure e) {
        throw xenbus_error("Cannot read port from " + xsd_port_path + ": " +
                std::string(std::strerror(errno)));
    }

    return port;
}

void lixs::xenbus::conn_dead(void)
{
}

