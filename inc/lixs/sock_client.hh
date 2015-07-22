#ifndef __LIXS_SOCK_CLIENT_HH__
#define __LIXS_SOCK_CLIENT_HH__

#include <lixs/client.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/sock_conn.hh>
#include <lixs/xenstore.hh>

#include <memory>


namespace lixs {

class sock_client : public client<sock_conn> {
public:
    sock_client(std::function<void(sock_client*)> dead_cb,
            xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io, int fd);
    ~sock_client();

private:
    static std::string get_id(void);

    void conn_dead(void);


    static unsigned int next_id;

    event_mgr& emgr;
    std::function<void(sock_client*)> dead_cb;
};

} /* namespace lixs */

#endif /* __LIXS_SOCK_CLIENT_HH__ */

