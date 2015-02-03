#include <lixs/events.hh>
#include <lixs/event_mgr.hh>
#include <lixs/sock_conn.hh>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::sock_conn::sock_conn(event_mgr& emgr, int fd)
    : alive(true), emgr(emgr)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    fd_cb_k::fd = fd;
    emgr.io_add(*this);
}

lixs::sock_conn::~sock_conn()
{
    emgr.io_remove(*this);
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
                emgr.io_set(*this);
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
                emgr.io_set(*this);
            }
        } else {
            /* need to wait */
            if (!ev_read) {
                ev_read = true;
                emgr.io_set(*this);
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
                emgr.io_set(*this);
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
                emgr.io_set(*this);
            }
        } else {
            /* need to wait */
            if (!ev_write) {
                ev_write = true;
                emgr.io_set(*this);
            }
        }
    }

    if (!alive) {
        conn_dead();
    }

    return done;
}

void lixs::sock_conn::operator()(bool read, bool write)
{
    process();
}

