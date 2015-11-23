#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/sock_client.hh>
#include <lixs/unix_sock_server.hh>
#include <lixs/xenstore.hh>

#include <cstddef>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::unix_sock_server::unix_sock_server(xenstore& xs, domain_mgr& dmgr,
        event_mgr& emgr, iomux& io,
        std::string rw_path, std::string ro_path)
    : xs(xs), dmgr(dmgr), emgr(emgr), io(io), rw_path(rw_path), ro_path(ro_path)
{
    struct sockaddr_un sock_addr = { 0 };
    sock_addr.sun_family = AF_UNIX;

    /* rw socket */
    rw_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, rw_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(rw_fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(rw_fd, 1);
    io.add(rw_fd, true, false, std::bind(&unix_sock_server::callback, this,
                std::placeholders::_1, std::placeholders::_2, rw_fd));

    /* ro socket */
    ro_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, ro_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(ro_fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(ro_fd, 1);
    io.add(ro_fd, true, false, std::bind(&unix_sock_server::callback, this,
                std::placeholders::_1, std::placeholders::_2, ro_fd));
}

lixs::unix_sock_server::~unix_sock_server(void)
{
    io.rem(rw_fd);
    close(rw_fd);
    unlink(rw_path.c_str());

    io.rem(ro_fd);
    close(ro_fd);
    unlink(ro_path.c_str());
}

void lixs::unix_sock_server::client_dead(sock_client* client)
{
    delete client;
}

void lixs::unix_sock_server::callback(bool read, bool write, int fd)
{
    std::function<void(sock_client*)> cb = std::bind(
            &unix_sock_server::client_dead, this, std::placeholders::_1);

    new sock_client(cb, xs, dmgr, emgr, io, accept(fd, NULL, NULL));
}

