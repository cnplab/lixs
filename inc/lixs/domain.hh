#ifndef __LIXS_DOMAIN_HH__
#define __LIXS_DOMAIN_HH__

#include <lixs/client.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

#include <cstring>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

template < class MAPPER >
class domain : public MAPPER, public client {
public:
    template < typename... ARGS >
    domain(xenstore& xs, event_mgr& emgr, domid_t domid, ARGS&&... args);

    ~domain();

    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);
    void process_events(bool read, bool write);
    bool is_alive(void);

protected:
    xc_evtchn *xce_handle;

    domid_t domid;
    evtchn_port_t local_port;
    evtchn_port_t remote_port;
    struct xenstore_domain_interface* interface;
};


template < typename MAPPER >
template < typename... ARGS >
domain<MAPPER>::domain(xenstore& xs, event_mgr& emgr, domid_t domid, ARGS&&... args)
    : MAPPER(domid, std::forward<ARGS>(args)...), client(xs, emgr), domid(domid)
{
    asprintf(&cid, "D%d", domid);
#ifdef DEBUG
    printf("%4s = new conn\n", cid);
#endif

    xs.get_domain_path(domid, abs_path);
    body = abs_path + strlen(abs_path);
    *body++ = '/';

    xce_handle = xc_evtchn_open(NULL, 0);
    interface = MAPPER::get();

    fd_cb.fd = xc_evtchn_fd(xce_handle);
    emgr.io_add(fd_cb);
}

template < typename MAPPER >
domain<MAPPER>::~domain()
{
    emgr.io_remove(fd_cb);

    xc_evtchn_close(xce_handle);

#ifdef DEBUG
    printf("%4s = closed conn\n", cid);
#endif
    free(cid);
}

template < typename MAPPER >
bool domain<MAPPER>::read(char*& buff, int& bytes)
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

    if (bytes > 0 && !fd_cb.ev_read) {
        fd_cb.ev_read = true;
        emgr.io_set(fd_cb);
    }

    if (bytes == 0 && fd_cb.ev_read) {
        fd_cb.ev_read = false;
        emgr.io_set(fd_cb);
    }

    if (len > 0) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

template < typename MAPPER >
bool domain<MAPPER>::write(char*& buff, int& bytes)
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

    if (bytes > 0 && !fd_cb.ev_write) {
        fd_cb.ev_write = true;
        emgr.io_set(fd_cb);
    }

    if (bytes == 0 && fd_cb.ev_write) {
        fd_cb.ev_write = false;
        emgr.io_set(fd_cb);
    }

    if (len > 0) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

template < typename MAPPER >
void domain<MAPPER>::process_events(bool read, bool write)
{
    evtchn_port_t port = xc_evtchn_pending(xce_handle);
    xc_evtchn_unmask(xce_handle, port);
}

template < typename MAPPER >
bool domain<MAPPER>::is_alive(void)
{
    return true;
}

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_HH__ */

