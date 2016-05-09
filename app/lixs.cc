#include "lixs_conf.hh"

#include <lixs/event_mgr.hh>
#include <lixs/mstore/store.hh>
#include <lixs/os_linux/epoll.hh>
#include <lixs/unix_sock_server.hh>
#include <lixs/os_linux/dom_exc.hh>
#include <lixs/xenbus.hh>
#include <lixs/xenstore.hh>

#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


static lixs::event_mgr* emgr_ptr;

static void signal_handler(int sig)
{
    if (sig == SIGINT) {
        printf("LiXS: Got SIGINT, stopping...\n");
        emgr_ptr->disable();
    }
}

static bool setup_logging(app::lixs_conf& conf)
{
    /* Reopen stdout first so that worst case we can still log to stderr */
    if (freopen(conf.log_file.c_str(), "w", stdout) == NULL) {
        goto out_err;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);

    if (freopen(conf.log_file.c_str(), "w", stderr) == NULL) {
        goto out_err;
    }
    setvbuf(stderr, NULL, _IOLBF, 0);

    return false;

out_err:
    fprintf(stderr, "LiXS: Failed to redirect output to file: %d\n", errno);
    return true;
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
        FILE* pidf = fopen(conf.pid_file.c_str(), "w");
        fprintf(pidf, "%d", getpid());
        fclose(pidf);
    }

    if (conf.log_to_file) {
        if (setup_logging(conf)) {
            return -1;
        }
    }

    if (conf.daemonize) {
        if (daemonize(conf)) {
            return -1;
        }
    }

    printf("LiXS: Starting server...\n");

    lixs::event_mgr emgr;
    lixs::os_linux::epoll epoll(emgr);
    lixs::mstore::store store;
    lixs::xenstore xs(store, emgr, epoll);

    lixs::domain_mgr dmgr(xs, emgr, epoll);

    lixs::unix_sock_server* nix = NULL;
    lixs::xenbus* xenbus = NULL;
    lixs::os_linux::dom_exc* dom_exc = NULL;

    if (conf.unix_sockets) {
        try {
            nix = new lixs::unix_sock_server(xs, dmgr, emgr, epoll,
                    conf.unix_socket_path, conf.unix_socket_ro_path);
        } catch (lixs::unix_sock_server_error& e) {
            printf("LiXS: [unix_sock_server] %s\n", e.what());
            goto out;
        }
    }

    if (conf.xenbus) {
        try {
            xenbus = new lixs::xenbus(xs, dmgr, emgr, epoll);
        } catch (lixs::xenbus_error& e) {
            printf("LiXS: [xenbus] %s\n", e.what());
            goto out;
        }
    }

    if (conf.virq_dom_exc) {
        try {
            dom_exc = new lixs::os_linux::dom_exc(xs, dmgr, epoll);
        } catch (lixs::os_linux::dom_exc_error& e) {
            printf("LiXS: [dom_exc] %s\n", e.what());
            goto out;
        }
    }


    emgr.enable();
    emgr_ptr = &emgr;
    signal(SIGINT, signal_handler);

    printf("LiXS: Entering main loop...\n");

    emgr.run();


out:
    if (nix) {
        delete nix;
    }

    if (xenbus) {
        delete xenbus;
    }

    if (dom_exc) {
        delete dom_exc;
    }

    printf("LiXS: Server stoped!\n");

    return 0;
}

