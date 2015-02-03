#ifndef __LIXS_UNIX_SOCK_SERVER_HH__
#define __LIXS_UNIX_SOCK_SERVER_HH__

#include <lixs/events.hh>
#include <lixs/event_mgr.hh>
#include <lixs/sock_client.hh>
#include <lixs/xenstore.hh>

#include <string>


namespace lixs {

class unix_sock_server {
public:
    unix_sock_server(xenstore& xs, event_mgr& emgr, std::string rw_path, std::string ro_path);
    ~unix_sock_server();


private:
    class fd_cb_k : public lixs::fd_cb_k {
    public:
        fd_cb_k (unix_sock_server& server)
            : server(server)
        { };

        void operator()(bool read, bool write);

        unix_sock_server& server;
    };


    void client_dead(sock_client* client);


    xenstore& xs;
    event_mgr& emgr;

    std::string rw_path;
    fd_cb_k rw_cb;

    std::string ro_path;
    fd_cb_k ro_cb;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_SOCK_SERVER_HH__ */

