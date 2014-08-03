#include <lixs/domain.hh>
#include <lixs/xen_server.hh>


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
    new domainU(xs, domid, port);
}

