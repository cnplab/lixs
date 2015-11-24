#ifndef __LIXS_UNIX_SOCK_SERVER_HH__
#define __LIXS_UNIX_SOCK_SERVER_HH__

#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/sock_client.hh>
#include <lixs/xenstore.hh>

#include <stdexcept>
#include <string>


namespace lixs {

class unix_sock_server_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class unix_sock_server {
public:
    unix_sock_server(xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io,
            const std::string& rw_path, const std::string& ro_path);
    ~unix_sock_server();

private:
    void callback(bool read, bool write, bool error, int fd);

private:
    int bind_socket(const std::string& path, std::string& err_msg);
    void client_dead(sock_client* client);

private:
    xenstore& xs;
    domain_mgr& dmgr;
    event_mgr& emgr;
    iomux& io;

    std::string rw_path;
    int rw_fd;

    std::string ro_path;
    int ro_fd;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_SOCK_SERVER_HH__ */

