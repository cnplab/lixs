#include <lixs/virq_handler.hh>


lixs::virq_handler::virq_handler(xenstore& xs, event_mgr& emgr)
    : xs(xs), emgr(emgr)
{
    xce_handle = xc_evtchn_open(NULL, 0);

    virq_port = xc_evtchn_bind_virq(xce_handle, VIRQ_DOM_EXC);

    fd = xc_evtchn_fd(xce_handle);
    ev_read = true;
    ev_write = false;
    emgr.io_add(*this);
}

lixs::virq_handler::~virq_handler()
{
    emgr.io_remove(*this);
    xc_evtchn_close(xce_handle);
}

void lixs::virq_handler::operator()(bool ev_read, bool ev_write)
{
    evtchn_port_t port = xc_evtchn_pending(xce_handle);
    xc_evtchn_unmask(xce_handle, port);

    /* TODO: Implement domain cleanup on dom_exc */
}

