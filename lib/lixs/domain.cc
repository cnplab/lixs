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

#include <lixs/domain.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/log/logger.hh>
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
    if (xcg_handle == NULL) {
        throw foreign_ring_mapper_error("Failed to open gnttab handle: " +
                std::string(std::strerror(errno)));
    }

    interface = (xenstore_domain_interface*) xc_gnttab_map_grant_ref(xcg_handle, domid,
            GNTTAB_RESERVED_XENSTORE, PROT_READ|PROT_WRITE);
    if (interface == NULL) {
        xc_gnttab_close(xcg_handle);
        throw foreign_ring_mapper_error("Failed to open gnttab handle: " +
                std::string(std::strerror(errno)));
    }
}

lixs::foreign_ring_mapper::~foreign_ring_mapper()
{
    xc_gnttab_munmap(xcg_handle, interface, 1);
    xc_gnttab_close(xcg_handle);
}


lixs::domain::domain(ev_cb dead_cb, xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io,
        log::logger& log, domid_t domid, evtchn_port_t port, unsigned int mfn)
    : client(get_id(domid), log, domid, xs, dmgr, log, io, domid, port, mfn),
    emgr(emgr), dead_cb(dead_cb), active(true), domid(domid)
{
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

void lixs::domain::conn_dead(void)
{
    emgr.enqueue_event(dead_cb);
}

std::string lixs::domain::get_id(domid_t domid)
{
    return "D" + std::to_string(domid);
}

