#ifndef __LIXS_OS_LINUX_DOM_EXC_HH__
#define __LIXS_OS_LINUX_DOM_EXC_HH__

#include <lixs/domain_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <stdexcept>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {
namespace os_linux {

class dom_exc_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class dom_exc {
public:
    dom_exc(xenstore& xs, domain_mgr& dmgr, iomux& io);
    virtual ~dom_exc();

    void callback(bool read, bool write, bool error);

private:
    xenstore& xs;
    domain_mgr& dmgr;
    iomux& io;

    int fd;

    bool alive;

    xc_interface* xc_handle;
    xc_evtchn *xce_handle;
    evtchn_port_t virq_port;
};

} /* namespace os_linux */
} /* namespace lixs */

#endif /* __LIXS_OS_LINUX_DOM_EXC_HH__ */

