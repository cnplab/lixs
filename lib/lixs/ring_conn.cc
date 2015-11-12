#include <lixs/ring_conn.hh>

#include <cerrno>
#include <cstring>

extern "C" {
#include <xenctrl.h>
#include <xen/io/xs_wire.h>
}


lixs::ring_conn_base::ring_conn_base(iomux& io, domid_t domid,
        evtchn_port_t port, xenstore_domain_interface* interface)
    : io(io), domid(domid), remote_port (port), interface(interface)
{
    xce_handle = xc_evtchn_open(NULL, 0);
    local_port = xc_evtchn_bind_interdomain(xce_handle, domid, remote_port);
    xc_evtchn_unmask(xce_handle, local_port);
    xc_evtchn_notify(xce_handle, local_port);

    fd = xc_evtchn_fd(xce_handle);
    io.add(*this);
}

lixs::ring_conn_base::~ring_conn_base()
{
    io.remove(*this);
    xc_evtchn_close(xce_handle);
}

bool lixs::ring_conn_base::read(char*& buff, int& bytes)
{
    bool notify = false;

    notify = read_chunck(buff, bytes);
    /*
     * If we're in the ring boundary and still have data to read we need to
     * recheck for space from the begin of the ring.
     */
    if (bytes > 0 && MASK_XENSTORE_IDX(interface->req_cons) == 0) {
        notify |= read_chunck(buff, bytes);
    }

    if (bytes > 0 && !ev_read) {
        ev_read = true;
        io.set(*this);
    }

    if (bytes == 0 && ev_read) {
        ev_read = false;
        io.set(*this);
    }

    if (notify) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

bool lixs::ring_conn_base::write(char*& buff, int& bytes)
{
    bool notify = false;

    notify = write_chunck(buff, bytes);
    /*
     * If we're in the ring boundary and still have data to write we need to
     * recheck for space from the begin of the ring.
     */
    if (bytes > 0 && MASK_XENSTORE_IDX(interface->rsp_prod) == 0) {
        notify |= write_chunck(buff, bytes);
    }

    if (bytes > 0 && !ev_write) {
        ev_write = true;
        io.set(*this);
    }

    if (bytes == 0 && ev_write) {
        ev_write = false;
        io.set(*this);
    }

    if (notify) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

void lixs::ring_conn_base::need_rx(void)
{
    if (!ev_read) {
        ev_read = true;
        io.set(*this);
    }
}

void lixs::ring_conn_base::need_tx(void)
{
    if (!ev_write) {
        ev_write = true;
        io.set(*this);
    }
}

void lixs::ring_conn_base::operator()(bool read, bool write)
{
    evtchn_port_t port = xc_evtchn_pending(xce_handle);
    xc_evtchn_unmask(xce_handle, port);

    if (read) {
        process_rx();
    }

    if (write) {
        process_tx();
    }
}

bool lixs::ring_conn_base::read_chunck(char*& buff, int& bytes)
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

bool lixs::ring_conn_base::write_chunck(char*&buff, int& bytes)
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

