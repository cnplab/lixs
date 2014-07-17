#include <lixs/unix_client.hh>
#include <lixs/iomux.hh>

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


lixs::unix_client::unix_client(lixs::iomux& io, int fd)
    : client(io), fd(fd), alive(true),
      iomux_ptr(handle, (void*) this), iomux_events(true, false)
{
    io.add(fd, iomux_events, &iomux_ptr);
}

lixs::unix_client::~unix_client()
{
    close(fd);
}

void lixs::unix_client::create(iomux& io, int fd)
{
    new unix_client(io, fd);
}

void lixs::unix_client::handle(iomux::ptr* ptr)
{
    lixs::unix_client* client = reinterpret_cast<lixs::unix_client*>(ptr->data);

    client->handle();

    if (!client->alive) {
        delete client;
    }
}

void lixs::unix_client::handle(void)
{
    socklen_t len;

    len = recv(fd, buff, sizeof(buff), 0);

    if (len == 0) {
        alive = false;
    } else {
        // TODO: pass this to XenStore
        buff[len] = '\0';
        fprintf(stdout, "%s", buff);
        fflush(stdout);
    }
}

