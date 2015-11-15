#ifndef __LIXS_UNIX_SOCK_SERVER_HH__
#define __LIXS_UNIX_SOCK_SERVER_HH__

#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/sock_client.hh>
#include <lixs/xenstore.hh>

#include <string>


namespace lixs {

class unix_sock_server {
public:
    unix_sock_server(xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io,
            std::string rw_path, std::string ro_path);
    ~unix_sock_server();

private:
    struct io_cb : public lixs::io_cb {
        io_cb (unix_sock_server& server)
            : server(server)
        { }

        void operator()(bool read, bool write);

        unix_sock_server& server;
    };

private:
    void client_dead(sock_client* client);

private:
    xenstore& xs;
    domain_mgr& dmgr;
    event_mgr& emgr;
    iomux& io;

    std::string rw_path;
    io_cb rw_cb;

    std::string ro_path;
    io_cb ro_cb;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_SOCK_SERVER_HH__ */

