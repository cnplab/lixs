#include <lixs/unix_client.hh>
#include <lixs/iomux.hh>
#include <lixs/store.hh>

#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


lixs::unix_client::unix_client(iomux& io, store& st, int fd)
    : client(io, st), fd(fd), events(false, false)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    io.add(*this, fd, events);
}

lixs::unix_client::~unix_client()
{
    close(fd);
}

void lixs::unix_client::create(iomux& io, store& st, int fd)
{
    new unix_client(io, st, fd);
}


bool lixs::unix_client::read(char*& buff, int& bytes)
{
    bool done;
    ssize_t len;

    if (!alive) {
        return false;
    }

    if (bytes == 0) {
        return true;
    }

    done = false;

    len = recv(fd, buff, bytes, 0);

    if (len == 0) {
        /* socket is closed */
        alive = false;
    } else if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* need to wait */
            if (!events.read) {
                events.read = true;
                io.set(*this, fd, events);
            }
        } else {
            /* error condition */
            alive = false;
        }
    } else {
        /* read successful */
        buff += len;
        bytes -= len;

        if (bytes  == 0) {
            done = true;

            if (events.read) {
                events.read = false;
                io.set(*this, fd, events);
            }
        }
    }

    return done;
}

bool lixs::unix_client::write(char*& buff, int& bytes)
{
    bool done;
    ssize_t len;

    if (!alive) {
        return false;
    }

    if (bytes == 0) {
        return true;
    }

    done = false;

    len = send(fd, buff, bytes, 0);

    if (len == 0) {
        /* socket is closed */
        alive = false;
    } else if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* need to wait */
            if (!events.write) {
                events.write = true;
                io.set(*this, fd, events);
            }
        } else {
            /* error condition */
            alive = false;
        }
    } else {
        /* write successful */
        buff += len;
        bytes -= len;

        if (bytes == 0) {
            done = true;

            if (events.write) {
                events.write = false;
                io.set(*this, fd, events);
            }
        }
    }

    return done;
}

