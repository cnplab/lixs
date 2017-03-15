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

#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/log/logger.hh>
#include <lixs/sock_client.hh>
#include <lixs/unix_sock_server.hh>
#include <lixs/xenstore.hh>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


lixs::unix_sock_server::unix_sock_server(const std::shared_ptr<xenstore>& xs,
        const std::shared_ptr<domain_mgr>& dmgr,
        const std::shared_ptr<event_mgr>& emgr,
        const std::shared_ptr<iomux>& io,
        const std::shared_ptr<log::logger>& log,
        const std::string& rw_path, const std::string& ro_path)
    : xs(xs), dmgr(dmgr), emgr(emgr), io(io), log(log), rw_path(rw_path), ro_path(ro_path),
    next_id(0)
{
    std::string err_msg;

    rw_fd = -1;
    ro_fd = -1;

    rw_fd = bind_socket(rw_path, err_msg);
    if (rw_fd == -1) {
        goto out_err;
    }

    ro_fd = bind_socket(ro_path, err_msg);
    if (ro_fd == -1) {
        goto out_err;
    }

    io->add(rw_fd, true, false, std::bind(&unix_sock_server::callback, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, rw_fd));
    io->add(ro_fd, true, false, std::bind(&unix_sock_server::callback, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ro_fd));

    return;

out_err:
    if (rw_fd != -1) {
        close(rw_fd);
        unlink(rw_path.c_str());
    }

    if (ro_fd != -1) {
        close(ro_fd);
        unlink(ro_path.c_str());
    }

    throw unix_sock_server_error(err_msg);
}

lixs::unix_sock_server::~unix_sock_server(void)
{
    for (auto& c : clients) {
        delete c.second;
    }
    clients.clear();

    io->rem(rw_fd);
    close(rw_fd);
    unlink(rw_path.c_str());

    io->rem(ro_fd);
    close(ro_fd);
    unlink(ro_path.c_str());
}

int lixs::unix_sock_server::bind_socket(const std::string& path, std::string& error_msg)
{
    int fd;
    int ret;

    struct sockaddr_un sock_addr = { 0 };
    sock_addr.sun_family = AF_UNIX;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        error_msg = "Failed to create socket: " + std::string(std::strerror(errno));
        return -1;
    }

    strncpy(sock_addr.sun_path, path.c_str(), sizeof(sock_addr.sun_path) - 1);

    ret = unlink(sock_addr.sun_path);
    if (ret == -1 && errno != ENOENT) {
        error_msg = "Failed to unlink socket: " + std::string(std::strerror(errno));
        return -1;
    }

    ret = bind(fd, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    if (ret == -1) {
        error_msg = "Failed to bind socket: " + std::string(std::strerror(errno));
        return -1;
    }

    ret = listen(fd, 1);
    if (ret == -1) {
        error_msg = "Failed to listen on socket: " + std::string(std::strerror(errno));
        return -1;
    }

    return fd;
}

void lixs::unix_sock_server::client_dead(long unsigned int id)
{
    client_map::iterator it;

    it = clients.find(id);
    if (it != clients.end()) {
        delete it->second;
        clients.erase(it);
    }
}

void lixs::unix_sock_server::callback(bool read, bool write, bool error, int fd)
{
    int client_fd;

    if (error) {
        log::LOG<log::level::WARN>::logf(*log,
                "[unix_socket_server] Got error from iomux");
        log::LOG<log::level::WARN>::logf(*log,
                "[unix_socket_server] Disabling socket (fd = %d)", fd);
        io->rem(fd);
        return;
    }

    client_fd = accept(fd, NULL, NULL);
    if (client_fd == -1) {
        log::LOG<log::level::ERROR>::logf(*log,
                "[unix_socket_server] Calling accept on socket failed: %s", std::strerror(errno));
        log::LOG<log::level::WARN>::logf(*log,
                "[unix_socket_server] Disabling socket (fd = %d)", fd);
        io->rem(fd);
        return;
    }

    long unsigned int id = next_id++;
    std::function<void(void)> cb = std::bind(&unix_sock_server::client_dead, this, id);

    sock_client* client = new sock_client(id, cb, *xs, *dmgr, *emgr, *io, *log, client_fd);

    clients.insert({id, client});
}

