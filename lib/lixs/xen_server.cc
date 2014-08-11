#include <lixs/domain.hh>
#include <lixs/xen_server.hh>

#include <map>
#include <utility>


lixs::xen_server::xen_server(xenstore& xs)
    : xs(xs)
{
    xs.set_xen_server(this);

    new domain0(xs);
}

lixs::xen_server::~xen_server(void)
{
}

void lixs::xen_server::create_domain(int domid, int port)
{
    domlist.insert(std::pair<int, lixs::domain*>(domid, new domainU(xs, domid, port)));
}

void lixs::xen_server::destroy_domain(int domid)
{
    std::map<int, lixs::domain*>::iterator it;

    it = domlist.find(domid);

    if (it != domlist.end()) {
        delete it->second;
        domlist.erase(it);
    }
}

