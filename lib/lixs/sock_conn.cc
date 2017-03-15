/*
 * LiXS: Lightweight XenStore
 *
 * Authors: Filipe Manco <filipe.manco@neclab.eu>
 *
 *
 * Copyright (c) 2016, NEC Europe Ltd., NEC Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

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


lixs::sock_conn::sock_conn(const std::shared_ptr<iomux>& io, int fd)
    : io(io), fd(fd), ev_read(false), ev_write(false), alive(true)
{
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1) {
        throw sock_conn_error("Unable to set O_NONBLOCK: " +
                std::string(std::strerror(errno)));
    }

    cb = std::shared_ptr<sock_conn_cb>(new sock_conn_cb(*this));

    io->add(fd, ev_read, ev_write, std::bind(sock_conn_cb::callback,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                std::weak_ptr<sock_conn_cb>(cb)));
}

lixs::sock_conn::~sock_conn()
{
    if (alive) {
        io->rem(fd);
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
                io->set(fd, ev_read, ev_write);
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
                io->set(fd, ev_read, ev_write);
            }
        } else {
            /* need to wait */
            if (!ev_read) {
                ev_read = true;
                io->set(fd, ev_read, ev_write);
            }
        }
    }

    if (!alive) {
        io->rem(fd);
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
                io->set(fd, ev_read, ev_write);
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
                io->set(fd, ev_read, ev_write);
            }
        } else {
            /* need to wait */
            if (!ev_write) {
                ev_write = true;
                io->set(fd, ev_read, ev_write);
            }
        }
    }

    if (!alive) {
        io->rem(fd);
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
        io->set(fd, ev_read, ev_write);
    }
}

void lixs::sock_conn::need_tx(void)
{
    if (!alive) {
        return;
    }

    if (!ev_write) {
        ev_write = true;
        io->set(fd, ev_read, ev_write);
    }
}

lixs::sock_conn_cb::sock_conn_cb(sock_conn& conn)
    : conn(conn)
{
}

void lixs::sock_conn_cb::callback(bool read, bool write, bool error,
        std::weak_ptr<sock_conn_cb> ptr)
{
    if (ptr.expired()) {
        return;
    }

    std::shared_ptr<sock_conn_cb> cb(ptr);

    if (!(cb->conn.alive)) {
        return;
    }

    if (error) {
        cb->conn.io->rem(cb->conn.fd);
        cb->conn.alive = false;
        cb->conn.conn_dead();
        return;
    }

    if (read) {
        cb->conn.process_rx();
    }

    if (write) {
        cb->conn.process_tx();
    }
}

