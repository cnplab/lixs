#include <lixs/iomux.hh>
#include <lixs/unix_server.hh>
#include <lixs/unix_client.hh>

#include <string>
#include <cstddef>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::unix_server::unix_server(lixs::iomux& io, std::string path, std::string ro_path)
    : server(io),
    sock_fd(-1), sock_path(path), iomux_ptr(handle_server, (void*) this),
    sock_ro_fd(-1), sock_ro_path(ro_path), iomux_ro_ptr(handle_server_ro, (void*) this)
{
    struct sockaddr_un sock_addr = { 0 };
    sock_addr.sun_family = AF_UNIX;

    /* rw socket */
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, sock_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(sock_fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(sock_fd, 1);
    io.add(sock_fd, iomux::events(true, false), &iomux_ptr);

    /* ro socket */
    sock_ro_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    strncpy(sock_addr.sun_path, sock_ro_path.c_str(), sizeof(sock_addr.sun_path) - 1);
    unlink(sock_addr.sun_path);
    bind(sock_ro_fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    listen(sock_ro_fd, 1);
    io.add(sock_ro_fd, iomux::events(true, false), &iomux_ro_ptr);
}

lixs::unix_server::~unix_server(void)
{
    close(sock_fd);
    unlink(sock_path.c_str());

    close(sock_ro_fd);
    unlink(sock_ro_path.c_str());
}


void lixs::unix_server::handle(int fd)
{
    unix_client::create(io, accept(fd, NULL, NULL));
}

void lixs::unix_server::handle_server(iomux::ptr* ptr)
{
    unix_server* server = reinterpret_cast<unix_server*>(ptr->data);
    server->handle(server->sock_fd);
}

void lixs::unix_server::handle_server_ro(iomux::ptr* ptr)
{
    unix_server* server = reinterpret_cast<unix_server*>(ptr->data);
    server->handle(server->sock_ro_fd);
}

