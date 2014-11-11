#include <lixs/domain.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern "C" {
#include <xenctrl.h>
#include <xen/grant_table.h>
}


lixs::domain::domain(xenstore& xs, int domid)
    : client(xs), domid(domid)
{
    asprintf(&cid, "D%d", domid);
    printf("%4s = new conn\n", cid);

    xs.get_domain_path(domid, abs_path);
    body = abs_path + strlen(abs_path);
    *body++ = '/';

    xcg_handle = xc_gnttab_open(NULL, 0);
    xce_handle = xc_evtchn_open(NULL, 0);

    fd_cb.fd = xc_evtchn_fd(xce_handle);
    xs.add(fd_cb);
}

lixs::domain::~domain()
{
    xs.remove(fd_cb);

    xc_gnttab_close(xcg_handle);
    xc_evtchn_close(xce_handle);

    printf("%4s = closed conn\n", cid);
    free(cid);
}

bool lixs::domain::read(char*& buff, int& bytes)
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
        xs.set(fd_cb);
    }

    if (bytes == 0 && fd_cb.ev_read) {
        fd_cb.ev_read = false;
        xs.set(fd_cb);
    }

    if (len > 0) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

bool lixs::domain::write(char*& buff, int& bytes)
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
        xs.set(fd_cb);
    }

    if (bytes == 0 && fd_cb.ev_write) {
        fd_cb.ev_write = false;
        xs.set(fd_cb);
    }

    if (len > 0) {
        xc_evtchn_notify(xce_handle, local_port);
    }

    return bytes == 0;
}

void lixs::domain::process_events(bool read, bool write)
{
    evtchn_port_t port = xc_evtchn_pending(xce_handle);
    xc_evtchn_unmask(xce_handle, port);
}

