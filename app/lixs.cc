#include <app/lixs/conf.hh>
#include <lixs/event_mgr.hh>
#include <lixs/mstore/store.hh>
#include <lixs/os_linux/epoll.hh>
#include <lixs/unix_sock_server.hh>
#include <lixs/virq_handler.hh>
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

    lixs::os_linux::epoll epoll;
    lixs::event_mgr emgr(epoll);
    lixs::mstore::store store;
    lixs::xenstore xs(store, emgr, epoll);

    lixs::domain_mgr dmgr(xs, emgr, epoll);

    lixs::unix_sock_server* nix = NULL;
    lixs::xenbus* xenbus = NULL;
    lixs::virq_handler* dom_exc = NULL;

    if (conf.unix_sockets) {
        nix = new lixs::unix_sock_server(xs, dmgr, emgr, epoll,
                conf.unix_socket_path, conf.unix_socket_ro_path);
    }

    if (conf.xenbus) {
        xenbus = new lixs::xenbus(xs, dmgr, emgr, epoll);
    }

    if (conf.virq_dom_exc) {
        dom_exc = new lixs::virq_handler(xs, dmgr, epoll);
    }


    emgr.enable();
    emgr_ptr = &emgr;
    signal(SIGINT, signal_handler);

    printf("LiXS: Entering main loop...\n");

    emgr.run();

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

