#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/sock_client.hh>
#include <lixs/unix_sock_server.hh>
#include <lixs/xenstore.hh>

#include <cstddef>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::unix_sock_server::unix_sock_server(xenstore& xs, domain_mgr& dmgr,
        event_mgr& emgr, iomux& io,
        std::string rw_path, std::string ro_path)
    : xs(xs), dmgr(dmgr), emgr(emgr), io(io),
    rw_path(rw_path), rw_cb(*this), ro_path(ro_path), ro_cb(*this)
{
    struct sockaddr_un sock_addr = { 0 };
    sock_addr.sun_family = AF_UNIX;

    /* rw socket */
    rw_cb.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, rw_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(rw_cb.fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(rw_cb.fd, 1);
    rw_cb.ev_read = true;
    rw_cb.ev_write = false;
    io.add(rw_cb);

    /* ro socket */
    ro_cb.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, ro_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(ro_cb.fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(ro_cb.fd, 1);
    ro_cb.ev_read = true;
    ro_cb.ev_write = false;
    io.add(ro_cb);
}

lixs::unix_sock_server::~unix_sock_server(void)
{
    io.remove(rw_cb);
    close(rw_cb.fd);
    unlink(rw_path.c_str());

    io.remove(ro_cb);
    close(ro_cb.fd);
    unlink(ro_path.c_str());
}

void lixs::unix_sock_server::client_dead(sock_client* client)
{
    delete client;
}

void lixs::unix_sock_server::io_cb::operator()(bool read, bool write)
{
    std::function<void(sock_client*)> cb = std::bind(
            &unix_sock_server::client_dead, &server, std::placeholders::_1);

    new sock_client(cb, server.xs, server.dmgr, server.emgr, server.io, accept(fd, NULL, NULL));
}

