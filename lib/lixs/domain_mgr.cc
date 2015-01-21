#include <lixs/domain.hh>
#include <lixs/event_mgr.hh>
#include <lixs/domain_mgr.hh>

#include <cerrno>
#include <map>
#include <utility>

extern "C" {
#include <xenctrl.h>
}


lixs::domain_mgr::domain_mgr(xenstore& xs, event_mgr& emgr)
    : xs(xs), emgr(emgr)
{
}

lixs::domain_mgr::~domain_mgr()
{
}

void lixs::domain_mgr::create_domain(domid_t domid, evtchn_port_t port, unsigned int mfn)
{
    domlist.insert(std::pair<domid_t, lixs::domain*>(domid, new domain(xs, emgr, domid, port, mfn)));
}

void lixs::domain_mgr::destroy_domain(domid_t domid)
{
    std::map<domid_t, lixs::domain*>::iterator it;

    it = domlist.find(domid);

    if (it != domlist.end()) {
        delete it->second;
        domlist.erase(it);
    }
}

void lixs::domain_mgr::exists_domain(domid_t domid, bool& exists)
{
    exists = (domlist.find(domid) != domlist.end());
}

