#ifndef __LIXS_RING_CONN_HH__
#define __LIXS_RING_CONN_HH__

#include <lixs/events.hh>
#include <lixs/iomux.hh>

#include <cerrno>
#include <cstring>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

template < typename MAPPER >
class ring_conn : public MAPPER, public fd_cb_k {
protected:
    template < typename... ARGS >
    ring_conn(iomux& io, domid_t domid, evtchn_port_t port, ARGS&&... args);
    virtual ~ring_conn();

    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);

    virtual void process(void) = 0;

private:
    void operator()(bool read, bool write);

    bool read_chunck(char*& buff, int& bytes);
    bool write_chunck(char*& buff, int& bytes);

private:
    iomux& io;

    xc_evtchn *xce_handle;

    domid_t domid;
    evtchn_port_t local_port;
    evtchn_port_t remote_port;
};


template < typename MAPPER >
template < typename... ARGS >
ring_conn<MAPPER>::ring_conn(iomux& io, domid_t domid, evtchn_port_t port, ARGS&&... args)
    : MAPPER(domid, std::forward<ARGS>(args)...), io(io),  domid(domid), remote_port(port)
{
    xce_handle = xc_evtchn_open(NULL, 0);
    local_port = xc_evtchn_bind_interdomain(xce_handle, domid, remote_port);
    xc_evtchn_unmask(xce_handle, local_port);
    xc_evtchn_notify(xce_handle, local_port);

    fd = xc_evtchn_fd(xce_handle);
    io.add(*this);
}

template < typename MAPPER >
ring_conn<MAPPER>::~ring_conn()
{
    io.remove(*this);
    xc_evtchn_close(xce_handle);
}

template < typename MAPPER >
bool ring_conn<MAPPER>::read(char*& buff, int& bytes)
{
    bool notify = false;

    notify = read_chunck(buff, bytes);
    /*
     * If we're in the ring boundary and still have data to read we need to
     * recheck for space from the begin of the ring.
     */
    if (bytes > 0 && MASK_XENSTORE_IDX(MAPPER::interface->req_cons) == 0) {
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

template < typename MAPPER >
bool ring_conn<MAPPER>::write(char*& buff, int& bytes)
{
    bool notify = false;

    notify = ring_conn<MAPPER>::write_chunck(buff, bytes);
    /*
     * If we're in the ring boundary and still have data to write we need to
     * recheck for space from the begin of the ring.
     */
    if (bytes > 0 && MASK_XENSTORE_IDX(MAPPER::interface->rsp_prod) == 0) {
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

template < typename MAPPER >
void ring_conn<MAPPER>::operator()(bool read, bool write)
{
    evtchn_port_t port = xc_evtchn_pending(xce_handle);
    xc_evtchn_unmask(xce_handle, port);

    process();
}

template < typename MAPPER >
bool ring_conn<MAPPER>::read_chunck(char*& buff, int& bytes)
{
    uint32_t len;
    XENSTORE_RING_IDX cons;
    XENSTORE_RING_IDX prod;

    cons = MAPPER::interface->req_cons;
    prod = MAPPER::interface->req_prod;
    xen_mb();

    len = XENSTORE_RING_SIZE - MASK_XENSTORE_IDX(cons);
    if ((prod - cons) < len) {
        len = prod - cons;
    }

    if (len > (unsigned int) bytes) {
        len = bytes;
    }

    memcpy(buff, MAPPER::interface->req + MASK_XENSTORE_IDX(cons), len);
    xen_mb();

    MAPPER::interface->req_cons += len;

    bytes -= len;
    buff += len;

    return len > 0;
}

template < typename MAPPER >
bool ring_conn<MAPPER>::write_chunck(char*&buff, int& bytes)
{
    uint32_t len;
    XENSTORE_RING_IDX cons;
    XENSTORE_RING_IDX prod;

    cons = MAPPER::interface->rsp_cons;
    prod = MAPPER::interface->rsp_prod;
    xen_mb();

    len = XENSTORE_RING_SIZE - MASK_XENSTORE_IDX(prod);
    if ((XENSTORE_RING_SIZE - (prod - cons)) < len) {
        len = XENSTORE_RING_SIZE - (prod - cons);
    }

    if (len > (unsigned int) bytes) {
        len = bytes;
    }

    memcpy(MAPPER::interface->rsp + MASK_XENSTORE_IDX(prod), buff, len);
    xen_mb();

    MAPPER::interface->rsp_prod += len;

    bytes -= len;
    buff += len;

    return len > 0;
}

} /* namespace lixs */

#endif /* __LIXS_RING_CONN_HH__ */

