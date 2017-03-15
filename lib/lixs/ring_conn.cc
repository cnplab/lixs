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

#include <lixs/ring_conn.hh>

#include <cerrno>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

extern "C" {
#include <xenctrl.h>
#include <xen/io/xs_wire.h>
}


lixs::ring_conn_base::ring_conn_base(const std::shared_ptr<iomux>& io, domid_t domid,
        evtchn_port_t port, xenstore_domain_interface* interface)
    : io(io), ev_read(false), ev_write(false), alive(true),
    domid(domid), remote_port(port), interface(interface)
{
    int ret;

    xce_handle = xc_evtchn_open(NULL, 0);
    if (xce_handle == NULL) {
        throw ring_conn_error("Failed to open evtchn handle: " +
                std::string(std::strerror(errno)));
    }

    local_port = xc_evtchn_bind_interdomain(xce_handle, domid, remote_port);
    if (local_port == (evtchn_port_t)(-1)) {
        xc_evtchn_close(xce_handle);
        throw ring_conn_error("Failed to bind evtchn: " +
                std::string(std::strerror(errno)));
    }

    ret = xc_evtchn_unmask(xce_handle, local_port);
    if (ret == -1) {
        xc_evtchn_close(xce_handle);
        throw ring_conn_error("Failed to unmask evtchn: " +
                std::string(std::strerror(errno)));
    }

    ret = xc_evtchn_notify(xce_handle, local_port);
    if (ret == -1) {
        xc_evtchn_close(xce_handle);
        throw ring_conn_error("Failed to notify evtchn: " +
                std::string(std::strerror(errno)));
    }

    fd = xc_evtchn_fd(xce_handle);

    cb = std::shared_ptr<ring_conn_cb>(new ring_conn_cb(*this));

    io->add(fd, ev_read, ev_write, std::bind(ring_conn_cb::callback,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                std::weak_ptr<ring_conn_cb>(cb)));
}

lixs::ring_conn_base::~ring_conn_base()
{
    if (alive) {
        io->rem(fd);
    }
    xc_evtchn_close(xce_handle);
}

bool lixs::ring_conn_base::read(char*& buff, int& bytes)
{
    int ret;
    bool notify = false;

    if (!alive) {
        return false;
    }

    notify = read_chunk(buff, bytes);
    /*
     * If we're in the ring boundary and still have data to read we need to
     * recheck for space from the begin of the ring.
     */
    if (bytes > 0 && MASK_XENSTORE_IDX(interface->req_cons) == 0) {
        notify |= read_chunk(buff, bytes);
    }

    if (bytes > 0 && !ev_read) {
        ev_read = true;
        io->set(fd, ev_read, ev_write);
    }

    if (bytes == 0 && ev_read) {
        ev_read = false;
        io->set(fd, ev_read, ev_write);
    }

    if (notify) {
        ret = xc_evtchn_notify(xce_handle, local_port);
        if (ret == -1) {
            alive = false;
            io->rem(fd);
            conn_dead();
        }
    }

    return alive && (bytes == 0);
}

bool lixs::ring_conn_base::write(char*& buff, int& bytes)
{
    int ret;
    bool notify = false;

    if (!alive) {
        return false;
    }

    notify = write_chunk(buff, bytes);
    /*
     * If we're in the ring boundary and still have data to write we need to
     * recheck for space from the begin of the ring.
     */
    if (bytes > 0 && MASK_XENSTORE_IDX(interface->rsp_prod) == 0) {
        notify |= write_chunk(buff, bytes);
    }

    if (bytes > 0 && !ev_write) {
        ev_write = true;
        io->set(fd, ev_read, ev_write);
    }

    if (bytes == 0 && ev_write) {
        ev_write = false;
        io->set(fd, ev_read, ev_write);
    }

    if (notify) {
        ret = xc_evtchn_notify(xce_handle, local_port);
        if (ret == -1) {
            alive = false;
            io->rem(fd);
            conn_dead();
        }
    }

    return alive && (bytes == 0);
}

void lixs::ring_conn_base::need_rx(void)
{
    if (!alive) {
        return;
    }

    if (!ev_read) {
        ev_read = true;
        io->set(fd, ev_read, ev_write);
    }
}

void lixs::ring_conn_base::need_tx(void)
{
    if (!alive) {
        return;
    }

    if (!ev_write) {
        ev_write = true;
        io->set(fd, ev_read, ev_write);
    }
}

lixs::ring_conn_cb::ring_conn_cb(ring_conn_base& conn)
    : conn(conn)
{
}

void lixs::ring_conn_cb::callback(bool read, bool write, bool error,
        std::weak_ptr<ring_conn_cb> ptr)
{
    int ret;
    evtchn_port_t port;

    if (ptr.expired()) {
        return;
    }

    std::shared_ptr<ring_conn_cb> cb(ptr);

    if (!(cb->conn.alive)) {
        return;
    }

    if (error) {
        goto out_err;
    }

    if (read) {
        cb->conn.process_rx();
    }

    if (write) {
        cb->conn.process_tx();
    }

    if (!(cb->conn.alive)) {
        return;
    }

    port = xc_evtchn_pending(cb->conn.xce_handle);
    if (port == (evtchn_port_t)(-1)) {
        goto out_err;
    }

    ret = xc_evtchn_unmask(cb->conn.xce_handle, port);
    if (ret == -1) {
        goto out_err;
    }

    return;

out_err:
    cb->conn.alive = false;
    cb->conn.io->rem(cb->conn.fd);
    cb->conn.conn_dead();
}

bool lixs::ring_conn_base::read_chunk(char*& buff, int& bytes)
{
    uint32_t len;
    XENSTORE_RING_IDX cons;
    XENSTORE_RING_IDX prod;

    cons = interface->req_cons;
    prod = interface->req_prod;
    xen_mb();

    len = XENSTORE_RING_SIZE - MASK_XENSTORE_IDX(cons);
    if ((prod - cons) < len) {
        len = prod - cons;
    }

    if (len > (unsigned int) bytes) {
        len = bytes;
    }

    memcpy(buff, interface->req + MASK_XENSTORE_IDX(cons), len);
    xen_mb();

    interface->req_cons += len;

    bytes -= len;
    buff += len;

    return len > 0;
}

bool lixs::ring_conn_base::write_chunk(char*&buff, int& bytes)
{
    uint32_t len;
    XENSTORE_RING_IDX cons;
    XENSTORE_RING_IDX prod;

    cons = interface->rsp_cons;
    prod = interface->rsp_prod;
    xen_mb();

    len = XENSTORE_RING_SIZE - MASK_XENSTORE_IDX(prod);
    if ((XENSTORE_RING_SIZE - (prod - cons)) < len) {
        len = XENSTORE_RING_SIZE - (prod - cons);
    }

    if (len > (unsigned int) bytes) {
        len = bytes;
    }

    memcpy(interface->rsp + MASK_XENSTORE_IDX(prod), buff, len);
    xen_mb();

    interface->rsp_prod += len;

    bytes -= len;
    buff += len;

    return len > 0;
}

