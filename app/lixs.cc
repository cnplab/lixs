#include <lixs/client.hh>
#include <lixs/epoll.hh>
#include <lixs/map_store.hh>
#include <lixs/unix_server.hh>
#include <lixs/xenstore.hh>
#include <lixs/xen_server.hh>

#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


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
    if (argc == 4) {
        daemon(1, 1);

        FILE* pidf = fopen(argv[2], "w");
        fprintf(pidf, "%d", getpid());
        fclose(pidf);

        fclose(stdin);
        freopen(argv[3], "w", stderr);
        freopen(argv[3], "w", stdout);
        setvbuf(stdout, NULL, _IOLBF, 0);
        setvbuf(stderr, NULL, _IOLBF, 0);
    }

    printf("========== LightWeIght XenStore ==========\n");

    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    lixs::epoll epoll;
    lixs::map_store store;
    lixs::xenstore xs(store, epoll);

    lixs::unix_server nix(xs, "/run/xenstored/socket", "/run/xenstored/socket_ro");
    lixs::xen_server xen(xs);

    server_stoped = false;
    while(!server_stoped) {
        xs.run();
    }

    return 0;
}

