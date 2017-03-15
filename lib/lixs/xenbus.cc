/*
 * LiXS: Lightweight XenStore
 *
 * Authors: Filipe Manco <filipe.manco@neclab.eu>
 *
 *
 * Copyright (c) 2016, NEC Europe Ltd., NEC Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/log/logger.hh>
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
    if (ptr == MAP_FAILED) {
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
lixs::xenbus::xenbus(const std::shared_ptr<xenstore>& xs,
        const std::shared_ptr<domain_mgr>& dmgr,
        const std::shared_ptr<event_mgr>& emgr,
        const std::shared_ptr<iomux>& io,
        const std::shared_ptr<log::logger>& log)
    : client("XB", log, 0, xs, dmgr, log, io, 0, xenbus_evtchn())
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
    } catch (std::ifstream::failure& e) {
        throw xenbus_error("Cannot read port from " + xsd_port_path + ": " +
                std::string(std::strerror(errno)));
    }

    return port;
}

void lixs::xenbus::conn_dead(void)
{
}

