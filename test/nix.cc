#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <string>

int tst_nix(void)
{
    int sock;
    std::string sock_path = "/run/lixssock";
    struct sockaddr_un sock_addr;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, sock_path.c_str(), sizeof(sock_addr.sun_path) - 1);

    connect(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr));
    write(sock, "lixs\n", 5);
    close(sock);

    return 0;
}

int main(int argc, char** argv)
{
    tst_nix();

    return 0;
}
