#ifndef __LIXS_SOCK_CLIENT_HH__
#define __LIXS_SOCK_CLIENT_HH__

#include <lixs/client.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/sock_conn.hh>
#include <lixs/xenstore.hh>
#include <lixs/xs_proto_v1/xs_proto.hh>

#include <memory>


namespace lixs {

class sock_client : public client<xs_proto_v1::xs_proto<sock_conn> > {
public:
    sock_client(long unsigned int id, std::function<void(sock_client*)> dead_cb,
            xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io, int fd);
    ~sock_client();

private:
    static std::string get_id(long unsigned int id);

    void conn_dead(void);

private:
    long unsigned int id;

    event_mgr& emgr;
    std::function<void(sock_client*)> dead_cb;
};

} /* namespace lixs */

#endif /* __LIXS_SOCK_CLIENT_HH__ */

