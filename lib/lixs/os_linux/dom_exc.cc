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
#include <lixs/iomux.hh>
#include <lixs/os_linux/dom_exc.hh>
#include <lixs/xenstore.hh>

#include <list>
#include <functional>


lixs::os_linux::dom_exc::dom_exc(xenstore& xs, domain_mgr& dmgr, iomux& io)
    : xs(xs), dmgr(dmgr), io(io), alive(true)
{
    xc_handle = xc_interface_open(NULL, NULL, 0);
    if (xc_handle == NULL) {
        throw dom_exc_error("Failed to open xc handle: " +
                std::string(std::strerror(errno)));
    }

    xce_handle = xc_evtchn_open(NULL, 0);
    if (xce_handle == NULL) {
        throw dom_exc_error("Failed to open evtchn handle: " +
                std::string(std::strerror(errno)));
    }

    virq_port = xc_evtchn_bind_virq(xce_handle, VIRQ_DOM_EXC);
    if (virq_port == (evtchn_port_t)(-1)) {
        xc_evtchn_close(xce_handle);
        throw dom_exc_error("Failed to bind virq: " +
                std::string(std::strerror(errno)));
    }

    fd = xc_evtchn_fd(xce_handle);
    io.add(fd, true, false, std::bind(&dom_exc::callback, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

lixs::os_linux::dom_exc::~dom_exc()
{
    if (alive) {
        io.rem(fd);
    }

    xc_evtchn_close(xce_handle);
    xc_interface_close(xc_handle);
}

void lixs::os_linux::dom_exc::callback(bool read, bool write, bool error)
{
    int ret;
    evtchn_port_t port;

    xc_dominfo_t dominfo;
    std::list<domid_t> dead_list;
    std::list<domid_t> dying_list;

    if (!alive) {
        return;
    }

    if (error) {
        goto out_err;
    }

    ret = 0;
    for (auto& d : dmgr) {
        domain* dom = d.second;
        domid_t domid = dom->get_domid();

        ret = xc_domain_getinfo(xc_handle, domid, 1, &dominfo);
        if (ret == -1) {
            break;
        } else if (ret != 1 || domid != dominfo.domid) {
            /* Domain doesn't exist already but is still in our list: remove */
            dead_list.push_back(domid);
        } else if (dominfo.dying) {
            /* Domain is dying: remove */
            dead_list.push_back(domid);
        } else if (dom->is_active() && (dominfo.shutdown || dominfo.crashed)) {
            dom->set_inactive();
            dying_list.push_back(domid);
        }
    }

    if (ret == -1) {
        goto out_err;
    }

    for (auto& d : dead_list) {
        if (dmgr.destroy(d) == 0) {
            xs.domain_release(d);
        }
    }

    for (auto& d : dying_list) {
        xs.domain_release(d);
    }

    port = xc_evtchn_pending(xce_handle);
    if (port == (evtchn_port_t)(-1)) {
        goto out_err;
    }

    ret = xc_evtchn_unmask(xce_handle, port);
    if (ret == -1) {
        goto out_err;
    }

    return;

out_err:
    alive = false;
    io.rem(fd);
}

