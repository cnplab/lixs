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

#include <lixs/log/logger.hh>

#include <getopt.h>
#include <string>


app::lixs_conf::lixs_conf(int argc, char** argv)
    : help(false),

    daemonize(false),
    write_pid_file(false),
    pid_file("/var/run/xenstored.pid"),
    log_to_file(false),
    log_file("/var/log/xen/lixs.log"),
    log_level(lixs::log::level::INFO),

    xenbus(false),
    virq_dom_exc(false),
    unix_sockets(false),
    unix_socket_path("/run/xenstored/socket"),
    unix_socket_ro_path("/run/xenstored/socket_ro"),

    error(false),

    cmd(argv[0])
{
    const char *short_opts = "hDL:xiu";
    const struct option long_opts[] = {
        { "help"               , no_argument       , NULL , 'h' },
        { "daemon"             , no_argument       , NULL , 'D' },
        /* NOTE: For now keep this required so that lixs is compatible with the
         * upstream sysV init script. If the argument is optional a value would
         * need to be specified with --pid-file=<argument> which doesn't happen
         * in the init script, where we find --pid-file <argument>.
         */
        { "pid-file"           , required_argument , NULL , 'p' },
        { "log-file"           , optional_argument , NULL , 'l' },
        { "log-level"          , required_argument , NULL,  'L' },
        { "xenbus"             , no_argument       , NULL , 'x' },
        { "virq-dom-exc"       , no_argument       , NULL , 'i' },
        { "unix-sockets"       , no_argument       , NULL , 'u' },
        { "socket-path"        , required_argument , NULL , 's' },
        { "socket_ro-path"     , required_argument , NULL , 'r' },
        { NULL , 0 , NULL , 0 }
    };

    int opt;
    int opt_index;
    std::string optarg_str;

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
                if (optarg) {
                    pid_file = std::string(optarg);
                }
                break;

            case 'l':
                log_to_file = true;
                if (optarg) {
                    log_file = std::string(optarg);
                }
                break;

            case 'L':
                optarg_str = std::string(optarg);

                if (optarg_str == "off") {
                    log_level = lixs::log::level::OFF;
                } else if (optarg_str == "error") {
                    log_level = lixs::log::level::ERROR;
                } else if (optarg_str == "warn") {
                    log_level = lixs::log::level::WARN;
                } else if (optarg_str == "info") {
                    log_level = lixs::log::level::INFO;
                } else if (optarg_str == "debug") {
                    log_level = lixs::log::level::DEBUG;
                } else if (optarg_str == "trace") {
                    log_level = lixs::log::level::TRACE;
                } else {
                    printf("Invalid log level %s\n", optarg);
                    error = true;
                }
                break;

            case 'x':
                xenbus = true;
                break;

            case 'i':
                virq_dom_exc = true;
                break;

            case 'u':
                unix_sockets = true;
                break;

            case 's':
                unix_socket_path = std::string(optarg);
                break;

            case 'r':
                unix_socket_ro_path = std::string(optarg);
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
    printf("      --pid-file=[file]  Write pid to file. Daemonizing automatically enables this\n"
           "                         option. Default value: '/var/run/xenstored.pid'.\n");
    printf("      --log-file <file>  Redirect output to file. Daemonizing automatically enables\n"
           "                         this options. Default value: '/var/log/xen/lixs.log'. This\n"
           "                         option is required for compatibility with the upstream sysV\n"
           "                         init script.\n");
    printf("      --log-level <level>\n"
           "                         Maximum log level. Default is info. Can be one of:\n"
           "                         [ off, error, warn, info, debug, trace ].\n");
    printf("\n");
    printf("Communication mechanisms:\n");
    printf("  -x, --xenbus           Enable communication with Linux's xenbus driver.\n");
    printf("  -i, --virq-dom-exc     Enable handling of VIRQ_DOM_EXC.\n");
    printf("  -u, --unix-sockets     Enable connections through unix sockets.\n");
    printf("      --socket-path <file>\n"
           "                         Read/write socket path. Default: /run/xenstored/socket.\n");
    printf("      --socket_ro-path <file>\n"
           "                         Read-only socket path. Default: /run/xenstored/socket_ro.\n");
}

