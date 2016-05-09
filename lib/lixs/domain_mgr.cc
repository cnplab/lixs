#include <lixs/domain.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>

#include <cerrno>
#include <utility>

extern "C" {
#include <xenctrl.h>
}


lixs::domain_mgr::domain_mgr(xenstore& xs, event_mgr& emgr, iomux& io)
    : xs(xs), emgr(emgr), io(io)
{
}

lixs::domain_mgr::~domain_mgr()
{
    for (auto& e : domains) {
        delete e.second;
    }
    domains.clear();
}

int lixs::domain_mgr::create(domid_t domid, evtchn_port_t port, unsigned int mfn)
{
    domain* dom;
    domain_map::iterator it;

    it = domains.find(domid);
    if (it == domains.end()) {
        std::function<void(void)> cb = std::bind(&domain_mgr::domain_dead, this, domid);

        try {
            dom = new domain(cb, xs, *this, emgr, io, domid, port, mfn);
        } catch (ring_conn_error& e) {
            printf("LiXS: [Domain %d] %s\n", domid, e.what());
            return ECANCELED;
        } catch (foreign_ring_mapper_error& e) {
            printf("LiXS: [Domain %d] %s\n", domid, e.what());
            return ECANCELED;
        }

        /* TODO: Consider using emplace when moving to gcc 4.8 is acceptable */
        domains.insert({domid, dom});

        return 0;
    } else {
        return EEXIST;
    }
}

int lixs::domain_mgr::destroy(domid_t domid)
{
    domain_map::iterator it;

    it = domains.find(domid);
    if (it != domains.end()) {
        delete it->second;
        domains.erase(it);

        return 0;
    } else {
        return ENOENT;
    }
}

void lixs::domain_mgr::exists(domid_t domid, bool& exists)
{
    exists = (domains.find(domid) != domains.end());
}

lixs::domain_mgr::iterator lixs::domain_mgr::begin()
{
    return domains.begin();
}

lixs::domain_mgr::iterator lixs::domain_mgr::end()
{
    return domains.end();
}

void lixs::domain_mgr::domain_dead(domid_t domid)
{
    domain_map::iterator it;

    it = domains.find(domid);
    if (it != domains.end()) {
        delete it->second;
        domains.erase(it);
    }
}

