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

#ifndef __LIXS_XENBUS_HH__
#define __LIXS_XENBUS_HH__

#include <lixs/client.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/log/logger.hh>
#include <lixs/ring_conn.hh>
#include <lixs/xenstore.hh>
#include <lixs/xs_proto_v1/xs_proto.hh>

#include <cerrno>
#include <stdexcept>
#include <string>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class xenbus_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class xenbus_mapper {
protected:
    xenbus_mapper(domid_t domid);
    virtual ~xenbus_mapper();

protected:
    xenstore_domain_interface* interface;

private:
    static const std::string xsd_kva_path;
};


class xenbus : public client<xs_proto_v1::xs_proto<ring_conn<xenbus_mapper> > > {
public:
    xenbus(const std::shared_ptr<xenstore>& xs,
            const std::shared_ptr<domain_mgr>& dmgr,
            const std::shared_ptr<event_mgr>& emgr,
            const std::shared_ptr<iomux>& io,
            const std::shared_ptr<log::logger>& log);
    ~xenbus();

private:
    static evtchn_port_t xenbus_evtchn(void);

    void conn_dead(void);

private:
    static const std::string xsd_port_path;
};

} /* namespace lixs */

#endif /* __LIXS_XENBUS_HH__ */

