#include <lixs/iomux.hh>
#include <lixs/sock_conn.hh>

#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::sock_conn::sock_conn(iomux& io, int fd)
    : io(io), alive(true)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    fd_cb_k::fd = fd;
    io.add(*this);
}

lixs::sock_conn::~sock_conn()
{
    io.remove(*this);
    close(fd);
}

bool lixs::sock_conn::read(char*& buff, int& bytes)
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
            if (!ev_read) {
                ev_read = true;
                io.set(*this);
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

            if (ev_read) {
                ev_read = false;
                io.set(*this);
            }
        } else {
            /* need to wait */
            if (!ev_read) {
                ev_read = true;
                io.set(*this);
            }
        }
    }

    if (!alive) {
        conn_dead();
    }

    return done;
}

bool lixs::sock_conn::write(char*& buff, int& bytes)
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
            if (!ev_write) {
                ev_write = true;
                io.set(*this);
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

            if (ev_write) {
                ev_write = false;
                io.set(*this);
            }
        } else {
            /* need to wait */
            if (!ev_write) {
                ev_write = true;
                io.set(*this);
            }
        }
    }

    if (!alive) {
        conn_dead();
    }

    return done;
}

void lixs::sock_conn::need_rx(void)
{
    if (!ev_read) {
        ev_read = true;
        io.set(*this);
    }
}

void lixs::sock_conn::need_tx(void)
{
    if (!ev_write) {
        ev_write = true;
        io.set(*this);
    }
}

void lixs::sock_conn::operator()(bool read, bool write)
{
    if (read) {
        process_rx();
    }

    if (write) {
        process_tx();
    }
}

