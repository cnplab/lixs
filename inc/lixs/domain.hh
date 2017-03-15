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

#ifndef __LIXS_DOMAIN_HH__
#define __LIXS_DOMAIN_HH__

#include <lixs/client.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/log/logger.hh>
#include <lixs/ring_conn.hh>
#include <lixs/xenstore.hh>
#include <lixs/xs_proto_v1/xs_proto.hh>

#include <cerrno>
#include <memory>
#include <sys/mman.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


namespace lixs {

class foreign_ring_mapper_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class foreign_ring_mapper {
protected:
    foreign_ring_mapper(domid_t domid, unsigned int mfn);
    virtual ~foreign_ring_mapper();

protected:
    struct xenstore_domain_interface* interface;

private:
    xc_gnttab *xcg_handle;
};


class domain : public client<xs_proto_v1::xs_proto<ring_conn<foreign_ring_mapper> > > {
public:
    domain(ev_cb dead_cb,
            const std::shared_ptr<xenstore>& xs,
            const std::shared_ptr<domain_mgr>& dmgr,
            const std::shared_ptr<event_mgr>& emgr,
            const std::shared_ptr<iomux>& io,
            const std::shared_ptr<log::logger>& log,
            domid_t domid, evtchn_port_t port, unsigned int mfn);
    ~domain();

    bool is_active(void);
    void set_inactive(void);
    domid_t get_domid(void);

private:
    static std::string get_id(domid_t domid);

    void conn_dead(void);

private:
    std::shared_ptr<event_mgr> emgr;
    ev_cb dead_cb;

    bool active;
    domid_t domid;
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_HH__ */

