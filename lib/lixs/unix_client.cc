#include <lixs/unix_client.hh>
#include <lixs/iomux.hh>

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


lixs::unix_client::unix_client(iomux& io, int fd)
    : client(io), fd(fd), alive(true), events(false, false)
{
    io.add(*this, fd, events);
}

lixs::unix_client::~unix_client()
{
    close(fd);
}

void lixs::unix_client::create(iomux& io, int fd)
{
    new unix_client(io, fd);
}


