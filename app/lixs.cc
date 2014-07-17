#include <lixs/epoll.hh>
#include <lixs/unix_server.hh>

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
    lixs::unix_server nix(epoll, "/run/lixssock", "/run/lixssock_ro");

    server_stoped = false;
    while(!server_stoped) {
        epoll.handle();
    }

    return 0;
}

