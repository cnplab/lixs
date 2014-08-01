#include <lixs/iomux.hh>
#include <lixs/unix_server.hh>
#include <lixs/unix_client.hh>
#include <lixs/xenstore.hh>

#include <string>
#include <cstddef>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::unix_server::unix_server(xenstore& xs, std::string rw_path, std::string ro_path)
    : xs(xs), rw_path(rw_path), rw_iok(*this), ro_path(ro_path), ro_iok(*this)
{
    struct sockaddr_un sock_addr = { 0 };
    sock_addr.sun_family = AF_UNIX;

    /* rw socket */
    rw_iok.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, rw_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(rw_iok.fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(rw_iok.fd, 1);
    xs.add(rw_iok, rw_iok.fd, fd_cb::fd_ev(true, false));

    /* ro socket */
    ro_iok.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, ro_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(ro_iok.fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(ro_iok.fd, 1);
    xs.add(ro_iok, ro_iok.fd, fd_cb::fd_ev(true, false));
}

lixs::unix_server::~unix_server(void)
{
    close(rw_iok.fd);
    unlink(rw_path.c_str());

    close(ro_iok.fd);
    unlink(ro_path.c_str());
}


void lixs::unix_server::handle(int fd)
{
    unix_client::create(xs, accept(fd, NULL, NULL));
}

