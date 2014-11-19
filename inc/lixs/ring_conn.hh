#ifndef __LIXS_RING_CONN_HH__
#define __LIXS_RING_CONN_HH__

#include <lixs/events.hh>
#include <lixs/event_mgr.hh>

#include <cerrno>
#include <cstring>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

template < typename MAPPER >
class ring_conn : public MAPPER, public fd_cb_k {
public:
    template < typename... ARGS >
    ring_conn(functor& cb, event_mgr& emgr, domid_t domid, evtchn_port_t port, ARGS&&... args);
    virtual ~ring_conn();

    void operator()(bool read, bool write);

    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);
    bool is_alive(void);

private:
    functor& cb;
    event_mgr& emgr;

    xc_evtchn *xce_handle;

    domid_t domid;
    evtchn_port_t local_port;
    evtchn_port_t remote_port;
};


template < typename MAPPER >
template < typename... ARGS >
ring_conn<MAPPER>::ring_conn(functor& cb, event_mgr& emgr, domid_t domid, evtchn_port_t port, ARGS&&... args)
    : MAPPER(domid, std::forward<ARGS>(args)...), cb(cb), emgr(emgr),  domid(domid), remote_port(port)
{
    xce_handle = xc_evtchn_open(NULL, 0);
    local_port = xc_evtchn_bind_interdomain(xce_handle, domid, remote_port);
    xc_evtchn_unmask(xce_handle, local_port);

    fd = xc_evtchn_fd(xce_handle);
    emgr.io_add(*this);
}

template < typename MAPPER >
ring_conn<MAPPER>::~ring_conn()
{
    xc_evtchn_close(xce_handle);
}

template < typename MAPPER >
void ring_conn<MAPPER>::operator()(bool read, bool write)
{
    evtchn_port_t port = xc_evtchn_pending(xce_handle);
    xc_evtchn_unmask(xce_handle, port);

    cb();
}

template < typename MAPPER >
bool ring_conn<MAPPER>::read(char*& buff, int& bytes)
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

    if (bytes > 0 && !ev_read) {
        ev_read = true;
        emgr.io_set(*this);
    }

    if (bytes == 0 && ev_read) {
        ev_read = false;
        emgr.io_set(*this);
    }

    if (len > 0) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

template < typename MAPPER >
bool ring_conn<MAPPER>::write(char*& buff, int& bytes)
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

    if (bytes > 0 && !ev_write) {
        ev_write = true;
        emgr.io_set(*this);
    }

    if (bytes == 0 && ev_write) {
        ev_write = false;
        emgr.io_set(*this);
    }

    if (len > 0) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

template < typename MAPPER >
bool ring_conn<MAPPER>::is_alive(void)
{
    return true;
}

} /* namespace lixs */

#endif /* __LIXS_RING_CONN_HH__ */

