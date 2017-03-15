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

#ifndef __LIXS_RING_CONN_HH__
#define __LIXS_RING_CONN_HH__

#include <lixs/iomux.hh>

#include <cerrno>
#include <memory>
#include <stdexcept>
#include <utility>

extern "C" {
#include <xenctrl.h>
#include <xen/io/xs_wire.h>
}


namespace lixs {

class ring_conn_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ring_conn_cb;

class ring_conn_base {
private:
    friend ring_conn_cb;

protected:
    ring_conn_base(const std::shared_ptr<iomux>& io, domid_t domid,
            evtchn_port_t port, xenstore_domain_interface* interface);
    virtual ~ring_conn_base();

protected:
    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);

    void need_rx(void);
    void need_tx(void);

    virtual void process_rx(void) = 0;
    virtual void process_tx(void) = 0;
    virtual void conn_dead(void) = 0;

private:
    bool read_chunk(char*& buff, int& bytes);
    bool write_chunk(char*& buff, int& bytes);

private:
    std::shared_ptr<iomux> io;

    int fd;
    bool ev_read;
    bool ev_write;

    bool alive;

    std::shared_ptr<ring_conn_cb> cb;

    domid_t domid;
    evtchn_port_t local_port;
    evtchn_port_t remote_port;

    xc_evtchn *xce_handle;
    xenstore_domain_interface* interface;
};

class ring_conn_cb {
public:
    ring_conn_cb(ring_conn_base& conn);

public:
    static void callback(bool read, bool write, bool error, std::weak_ptr<ring_conn_cb> ptr);

private:
    ring_conn_base& conn;
};


template < typename MAPPER >
class ring_conn : public MAPPER, public ring_conn_base {
protected:
    template < typename... ARGS >
    ring_conn(const std::shared_ptr<iomux>& io, domid_t domid, evtchn_port_t port, ARGS&&... args);
    virtual ~ring_conn();
};


template < typename MAPPER >
template < typename... ARGS >
ring_conn<MAPPER>::ring_conn(const std::shared_ptr<iomux>& io,
        domid_t domid, evtchn_port_t port, ARGS&&... args)
    : MAPPER(domid, std::forward<ARGS>(args)...),
    ring_conn_base(io, domid, port, MAPPER::interface)
{
}

template < typename MAPPER >
ring_conn<MAPPER>::~ring_conn()
{
}

} /* namespace lixs */

#endif /* __LIXS_RING_CONN_HH__ */

