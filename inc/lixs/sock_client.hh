#ifndef __LIXS_SOCK_CLIENT_HH__
#define __LIXS_SOCK_CLIENT_HH__

#include <lixs/client.hh>
#include <lixs/event_mgr.hh>
#include <lixs/sock_conn.hh>
#include <lixs/xenstore.hh>


namespace lixs {

class sock_client : public client<sock_conn> {
public:
    static void create(xenstore& xs, event_mgr& emgr, int fd);

private:
    sock_client(xenstore& xs, event_mgr& emgr, int fd);
    ~sock_client();
};

} /* namespace lixs */

#endif /* __LIXS_SOCK_CLIENT_HH__ */

