#ifndef __LIXS_UNIX_CLIENT_HH__
#define __LIXS_UNIX_CLIENT_HH__

#include <lixs/client.hh>
#include <lixs/event_mgr.hh>
#include <lixs/sock_conn.hh>
#include <lixs/xenstore.hh>


namespace lixs {

class unix_client : public client<sock_conn> {
public:
    static void create(xenstore& xs, event_mgr& emgr, int fd);

private:
    unix_client(xenstore& xs, event_mgr& emgr, int fd);
    ~unix_client();
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_CLIENT_HH__ */

