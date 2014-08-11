#ifndef __LIXS_XEN_SERVER_HH__
#define __LIXS_XEN_SERVER_HH__

#include <lixs/domain.hh>
#include <lixs/events.hh>
#include <lixs/server.hh>
#include <lixs/xenstore.hh>

#include <map>


namespace lixs {

class domain;

class xen_server : public server {
public:
    xen_server(xenstore& xs);
    ~xen_server(void);

    void create_domain(int domid, int port);

private:
    xenstore& xs;

    std::map<int, lixs::domain*> domlist;
};

} /* namespace lixs */

#endif /* __LIXS_XEN_SERVER_HH__ */

