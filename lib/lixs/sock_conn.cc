#include <lixs/iomux.hh>
#include <lixs/sock_conn.hh>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::sock_conn::sock_conn(iomux& io, int fd)
    : io(io), fd(fd), ev_read(false), ev_write(false), alive(true)
{
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        throw sock_conn_error("Unable to set O_NONBLOCK: " +
                std::string(std::strerror(errno)));
    }

    cb = std::shared_ptr<sock_conn_cb>(new sock_conn_cb(*this));

    io.add(fd, ev_read, ev_write, std::bind(sock_conn_cb::callback,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                std::weak_ptr<sock_conn_cb>(cb)));
}

lixs::sock_conn::~sock_conn()
{
    if (alive) {
        io.rem(fd);
    }
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
                io.set(fd, ev_read, ev_write);
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
                io.set(fd, ev_read, ev_write);
            }
        } else {
            /* need to wait */
            if (!ev_read) {
                ev_read = true;
                io.set(fd, ev_read, ev_write);
            }
        }
    }

    if (!alive) {
        io.rem(fd);
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

    len = send(fd, buff, bytes, MSG_NOSIGNAL);

    if (len == 0) {
        /* socket is closed */
        alive = false;
    } else if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* need to wait */
            if (!ev_write) {
                ev_write = true;
                io.set(fd, ev_read, ev_write);
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
                io.set(fd, ev_read, ev_write);
            }
        } else {
            /* need to wait */
            if (!ev_write) {
                ev_write = true;
                io.set(fd, ev_read, ev_write);
            }
        }
    }

    if (!alive) {
        io.rem(fd);
        conn_dead();
    }

    return done;
}

void lixs::sock_conn::need_rx(void)
{
    if (!alive) {
        return;
    }

    if (!ev_read) {
        ev_read = true;
        io.set(fd, ev_read, ev_write);
    }
}

void lixs::sock_conn::need_tx(void)
{
    if (!alive) {
        return;
    }

    if (!ev_write) {
        ev_write = true;
        io.set(fd, ev_read, ev_write);
    }
}

lixs::sock_conn_cb::sock_conn_cb(sock_conn& conn)
    : conn(conn)
{
}

void lixs::sock_conn_cb::callback(bool read, bool write, bool error,
        std::weak_ptr<sock_conn_cb> ptr)
{
    if (error) {
        /* FIXME: handle error */
        return;
    }

    if (ptr.expired()) {
        return;
    }

    std::shared_ptr<sock_conn_cb> cb(ptr);

    if (!(cb->conn.alive)) {
        return;
    }

    if (read) {
        cb->conn.process_rx();
    }

    if (write) {
        cb->conn.process_tx();
    }
}

