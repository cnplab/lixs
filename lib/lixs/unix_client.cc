#include <lixs/unix_client.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


lixs::unix_client::unix_client(xenstore& xs, int fd)
    : client(xs)
{
    asprintf(&cid, "C%d", fd);
#ifdef DEBUG
    printf("%4s = new conn\n", cid);
#endif

    body = abs_path;

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    fd_cb.fd = fd;
    xs.add(fd_cb);
}

lixs::unix_client::~unix_client()
{
    xs.remove(fd_cb);
    close(fd_cb.fd);

#ifdef DEBUG
    printf("%4s = closed conn\n", cid);
#endif
    free(cid);
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

    len = recv(fd_cb.fd, buff, bytes, 0);

    if (len == 0) {
        /* socket is closed */
        alive = false;
    } else if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* need to wait */
            if (!fd_cb.ev_read) {
                fd_cb.ev_read = true;
                xs.set(fd_cb);
            }
        } else {
            /* error condition */
            alive = false;
        }
    } else {
        /* read successful */
        buff += len;
        bytes -= len;

        if (bytes == 0) {
            done = true;

            if (fd_cb.ev_read) {
                fd_cb.ev_read = false;
                xs.set(fd_cb);
            }
        } else {
            /* need to wait */
            if (!fd_cb.ev_read) {
                fd_cb.ev_read = true;
                xs.set(fd_cb);
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

    len = send(fd_cb.fd, buff, bytes, 0);

    if (len == 0) {
        /* socket is closed */
        alive = false;
    } else if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* need to wait */
            if (!fd_cb.ev_write) {
                fd_cb.ev_write = true;
                xs.set(fd_cb);
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

            if (fd_cb.ev_write) {
                fd_cb.ev_write = false;
                xs.set(fd_cb);
            }
        } else {
            /* need to wait */
            if (!fd_cb.ev_write) {
                fd_cb.ev_write = true;
                xs.set(fd_cb);
            }
        }
    }

    return done;
}

