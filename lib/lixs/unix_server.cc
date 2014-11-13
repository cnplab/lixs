#include <lixs/iomux.hh>
#include <lixs/unix_server.hh>
#include <lixs/unix_client.hh>
#include <lixs/xenstore.hh>

#include <cstddef>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::unix_server::unix_server(xenstore& xs, std::string rw_path, std::string ro_path)
    : xs(xs), rw_path(rw_path), rw_cb(*this), ro_path(ro_path), ro_cb(*this)
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
    xs.add(rw_cb);

    /* ro socket */
    ro_cb.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, ro_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(ro_cb.fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(ro_cb.fd, 1);
    ro_cb.ev_read = true;
    ro_cb.ev_write = false;
    xs.add(ro_cb);
}

lixs::unix_server::~unix_server(void)
{
    close(rw_cb.fd);
    unlink(rw_path.c_str());

    close(ro_cb.fd);
    unlink(ro_path.c_str());
}


void lixs::unix_server::fd_cb_k::operator()(bool read, bool write)
{
    unix_client::create(server.xs, accept(fd, NULL, NULL));
}

