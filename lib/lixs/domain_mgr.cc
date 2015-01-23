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

void lixs::domain_mgr::create(domid_t domid, evtchn_port_t port, unsigned int mfn)
{
    /* TODO: Consider using emplace when moving to gcc 4.8 is acceptable */

    domains.insert(std::make_pair(domid, new domain(xs, emgr, domid, port, mfn)));
}

void lixs::domain_mgr::destroy(domid_t domid)
{
    domain_map::iterator it;

    it = domains.find(domid);
    if (it != domains.end()) {
        delete it->second;
        domains.erase(it);
    }
}

void lixs::domain_mgr::exists(domid_t domid, bool& exists)
{
    exists = (domains.find(domid) != domains.end());
}

