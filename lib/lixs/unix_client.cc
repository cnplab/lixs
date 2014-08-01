#include <lixs/unix_client.hh>
#include <lixs/xenstore.hh>

#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


lixs::unix_client::unix_client(xenstore& xs, int fd)
    : client(xs), fd(fd), events(false, false)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    xs.add(*this, fd, events);
}

lixs::unix_client::~unix_client()
{
    xs.remove(fd);
    close(fd);
}

void lixs::unix_client::create(xenstore& xs, int fd)
{
    new unix_client(xs, fd);
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
                xs.set(*this, fd, events);
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
                xs.set(*this, fd, events);
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
                xs.set(*this, fd, events);
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
                xs.set(*this, fd, events);
            }
        }
    }

    return done;
}

