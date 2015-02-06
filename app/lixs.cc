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
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


struct lixs_conf {
    lixs_conf(int argc, char** argv)
        : error(false),
        help(false),
        daemonize(false),
        log_to_file(false),
        write_pid_file(false),

        pid_file("/var/run/xenstored.pid"),
        log_file("/var/log/xen/lixs.log"),
        unix_socket_path("/run/xenstored/socket"),
        unix_socket_ro_path("/run/xenstored/socket_ro"),
        cmd(argv[0])
    {
        /* FIXME: allow to specify socket paths */
        /* TODO: Improve configurability */

        const char *short_opts = "hD";
        const struct option long_opts[] = {
            { "help"               , no_argument       , NULL , 'h' },
            { "daemon"             , no_argument       , NULL , 'D' },
            { "pid-file"           , required_argument , NULL , 'p' },
            { "log-file"           , required_argument , NULL , 'l' },
            { NULL , 0 , NULL , 0 }
        };

        int opt;
        int opt_index;

        while (1) {
            opt = getopt_long(argc, argv, short_opts, long_opts, &opt_index);

            if (opt == -1) {
                break;
            }

            switch (opt) {
                case 'h':
                    help = true;
                    break;

                case 'D':
                    daemonize = true;
                    log_to_file = true;
                    write_pid_file = true;
                    break;

                case 'p':
                    write_pid_file = true;
                    pid_file = std::string(optarg);
                    break;

                case 'l':
                    log_to_file = true;
                    log_file = std::string(optarg);
                    break;

                default:
                    error = true;
                    break;
            }
        }

        while (optind < argc) {
            error = true;

            printf("%s: invalid argument \'%s\'\n", cmd.c_str(), argv[optind]);
            optind++;
        }
    }

    void print_usage() {
        printf("Usage: %s [OPTION]...\n", cmd.c_str());
        printf("\n");
        printf("Options:\n");
        printf("  -D, --daemon           Run in background\n");
        printf("      --pid-file <file>  Write process pid to file\n");
        printf("                         [default: /var/run/xenstored.pid]\n");
        printf("      --log-file <file>  Write log output to file\n");
        printf("                         [default: /var/log/xen/lixs.log]\n");
        printf("  -h, --help             Display this help and exit\n");
    }


    bool error;

    bool help;
    bool daemonize;
    bool log_to_file;
    bool write_pid_file;

    std::string pid_file;
    std::string log_file;
    std::string unix_socket_path;
    std::string unix_socket_ro_path;

private:
    std::string cmd;
};

static bool server_stoped;

static void signal_handler(int sig)
{
    if (sig == SIGINT) {
        printf("lixs: got SIGINT\n");
        server_stoped = true;
    }
}

int main(int argc, char** argv)
{
    lixs_conf conf(argc, argv);

    if (conf.error) {
        conf.print_usage();
        return -1;
    }

    if (conf.help) {
        conf.print_usage();
        return 0;
    }

    if (conf.log_to_file) {
        fclose(stdin);
        freopen(conf.log_file.c_str(), "w", stderr);
        freopen(conf.log_file.c_str(), "w", stdout);
        setvbuf(stdout, NULL, _IOLBF, 0);
        setvbuf(stderr, NULL, _IOLBF, 0);
    }

    if (conf.write_pid_file) {
        FILE* pidf = fopen(conf.pid_file.c_str(), "w");
        fprintf(pidf, "%d", getpid());
        fclose(pidf);
    }

    if (conf.daemonize) {
        daemon(1, 1);
    }

    printf("========== LightWeIght XenStore ==========\n");

    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    lixs::os_linux::epoll epoll;
    lixs::event_mgr emgr(epoll);
    lixs::mstore::store store;
    lixs::xenstore xs(store, emgr);

    lixs::unix_sock_server nix(xs, emgr, conf.unix_socket_path, conf.unix_socket_ro_path);
    lixs::virq_handler dom_exc(xs, epoll);
    lixs::xenbus xenbus(xs, emgr);

    server_stoped = false;
    while(!server_stoped) {
        emgr.run();
    }

    return 0;
}

