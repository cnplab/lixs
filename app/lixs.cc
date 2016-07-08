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

#include "lixs_conf.hh"

#include <lixs/event_mgr.hh>
#include <lixs/log/logger.hh>
#include <lixs/mstore/store.hh>
#include <lixs/os_linux/epoll.hh>
#include <lixs/unix_sock_server.hh>
#include <lixs/os_linux/dom_exc.hh>
#include <lixs/xenbus.hh>
#include <lixs/xenstore.hh>

#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>


using lixs::log::level;
using lixs::log::LOG;

static lixs::log::logger* log_ptr = NULL;
static lixs::event_mgr* emgr_ptr = NULL;

static void signal_handler(int sig)
{
    if (sig == SIGINT) {
        LOG<level::INFO>::logf(*log_ptr, "Got SIGINT, stopping...");
        emgr_ptr->disable();
    }
}

static void setup_signal_handler(lixs::event_mgr& emgr, lixs::log::logger& log)
{
    emgr_ptr = &emgr;
    log_ptr = &log;

    signal(SIGINT, signal_handler);
}

static int daemonize(app::lixs_conf& conf)
{
    /* If log to file is enabled we cannot let daemon() handle file descriptors
     * (it would close the log files) but we still need to close stdin
     */
    if (conf.log_to_file) {
        fclose(stdin);
    }

    if (daemon(1, conf.log_to_file ? 1 : 0)) {
        fprintf(stderr, "LiXS: Failed to daemonize: %d\n", errno);
        return true;
    }

    return false;
}

static int create_pid_file(const std::string& pid_file)
{
    int ret;
    FILE* pidf;

    pidf = fopen(pid_file.c_str(), "w");

    if (pidf == NULL) {
        fprintf(stderr, "LiXS: Failed to open PID file: %s\n", std::strerror(errno));
        return -1;
    }

    ret = fprintf(pidf, "%d", getpid());
    if (ret < 0) {
        fprintf(stderr, "LiXS: Failed to write to PID file\n");
        return ret;
    }

    fclose(pidf);

    return 0;
}

static int remove_pid_file(const std::string& pid_file)
{
    int ret;

    ret = unlink(pid_file.c_str());
    if (ret) {
        /* This print might fail if we daemonized and stderr is closed, but
         * lets leave it at that for now.
         */
        fprintf(stderr, "LiXS: Failed to remove PID file\n");
    }

    return ret;
}

int main(int argc, char** argv)
{
    app::lixs_conf conf(argc, argv);

    if (conf.error) {
        conf.print_usage();
        return -1;
    }

    if (conf.help) {
        conf.print_usage();
        return 0;
    }

    if (conf.write_pid_file) {
        if (create_pid_file(conf.pid_file)) {
            return -1;
        }
    }

    if (conf.daemonize) {
        if (daemonize(conf)) {
            return -1;
        }
    }

    std::unique_ptr<lixs::log::logger> log;

    try {
        if (conf.log_to_file) {
            log = std::unique_ptr<lixs::log::logger>(
                    new lixs::log::logger(conf.log_level, conf.log_file));
        } else {
            log = std::unique_ptr<lixs::log::logger>(
                    new lixs::log::logger(conf.log_level));
        }
    } catch (std::runtime_error& e) {
        printf("LiXS: [logger] %s\n", e.what());
        return -1;
    }

    lixs::event_mgr emgr;
    lixs::os_linux::epoll epoll(emgr);
    lixs::mstore::store store(*log);
    lixs::xenstore xs(store, emgr, epoll);

    lixs::domain_mgr dmgr(xs, emgr, epoll, *log);

    std::unique_ptr<lixs::unix_sock_server> nix;
    std::unique_ptr<lixs::xenbus> xenbus;
    std::unique_ptr<lixs::os_linux::dom_exc> dom_exc;

    if (conf.unix_sockets) {
        try {
            nix = std::unique_ptr<lixs::unix_sock_server>(
                    new lixs::unix_sock_server(xs, dmgr, emgr, epoll, *log,
                        conf.unix_socket_path, conf.unix_socket_ro_path));
        } catch (lixs::unix_sock_server_error& e) {
            printf("LiXS: [unix_sock_server] %s\n", e.what());
            return -1;
        }
    }

    if (conf.xenbus) {
        try {
            xenbus = std::unique_ptr<lixs::xenbus>(
                    new lixs::xenbus(xs, dmgr, emgr, epoll, *log));
        } catch (lixs::xenbus_error& e) {
            printf("LiXS: [xenbus] %s\n", e.what());
            return -1;
        }
    }

    if (conf.virq_dom_exc) {
        try {
            dom_exc = std::unique_ptr<lixs::os_linux::dom_exc>(
                    new lixs::os_linux::dom_exc(xs, dmgr, epoll));
        } catch (lixs::os_linux::dom_exc_error& e) {
            printf("LiXS: [dom_exc] %s\n", e.what());
            return -1;
        }
    }


    LOG<level::INFO>::logf(*log, "Starting server...");

    emgr.enable();

    setup_signal_handler(emgr, *log);

    emgr.run();

    LOG<level::INFO>::logf(*log, "Server stoped!");

    remove_pid_file(conf.pid_file);

    return 0;
}

