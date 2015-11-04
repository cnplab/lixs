#include <app/lixs/conf.hh>

#include <getopt.h>
#include <string>


app::lixs_conf::lixs_conf(int argc, char** argv)
    : help(false),

    daemonize(false),
    write_pid_file(false),
    pid_file("/var/run/xenstored.pid"),
    log_to_file(false),
    log_file("/var/log/xen/lixs.log"),

    xenbus(false),
    virq_dom_exc(false),
    unix_socket_path("/run/xenstored/socket"),
    unix_socket_ro_path("/run/xenstored/socket_ro"),

    error(false),

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
        { "xenbus"             , no_argument       , NULL , 'x' },
        { "virq-dom-exc"       , no_argument       , NULL , 'i' },
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

            case 'x':
                xenbus = true;
                break;

            case 'i':
                virq_dom_exc = true;
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

void app::lixs_conf::print_usage() {
    printf("Usage: %s [OPTION]...\n", cmd.c_str());
    printf("\n");
    printf("  -h, --help             Display this help and exit.\n");
    printf("\n");
    printf("General configuration:\n");
    printf("  -D, --daemon           Run in application in background.\n");
    printf("      --pid-file <file>  Write pid to file. Daemonizing automatically enables this\n"
           "                         option. Default value: '/var/run/xenstored.pid'.\n");
    printf("      --log-file <file>  Redirect output to file. Daemonizing automatically enables\n"
           "                         this options. Default value: '/var/log/xen/lixs.log'.\n");
    printf("\n");
    printf("Communication mechanisms:\n");
    printf("      --xenbus           Enable communication with Linux's xenbus driver.\n");
    printf("      --virq-dom-exc     Enable handling of VIRQ_DOM_EXC.\n");
}
