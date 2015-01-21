#ifndef __LIXS_VIRQ_HANDLER_HH__
#define __LIXS_VIRQ_HANDLER_HH__

#include <lixs/events.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

#include <cerrno>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class virq_handler : fd_cb_k {
public:
    virq_handler(xenstore& xs, event_mgr& emgr);
    ~virq_handler();

    void operator()(bool ev_read, bool ev_write);

private:
    xenstore& xs;
    event_mgr& emgr;

    xc_evtchn *xce_handle;
    evtchn_port_t virq_port;
};

} /* namespace lixs */

#endif /* __LIXS_VIRQ_HANDLER_HH__ */

