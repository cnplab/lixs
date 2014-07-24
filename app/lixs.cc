#include <lixs/epoll.hh>
#include <lixs/map_store.hh>
#include <lixs/unix_server.hh>
#include <lixs/xen_server.hh>

#include <csignal>
#include <cstdio>


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
    printf("========== LightWeIght XenStore ==========\n");

    signal(SIGINT, signal_handler);

    lixs::epoll epoll;
    lixs::map_store store;

    lixs::unix_server nix(epoll, store, "/run/lixssock", "/run/lixssock_ro");
    lixs::xen_server xen(epoll);

    server_stoped = false;
    while(!server_stoped) {
        epoll.handle();
    }

    return 0;
}

