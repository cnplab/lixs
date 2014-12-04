#include <cstdio>
#include <cstdlib>

extern "C" {
#include <xenstore.h>
}

int main(int argc, char** argv)
{
    int domid = 0;
    struct xs_handle *xs = xs_daemon_open();

    if (argc > 1) {
        domid = atoi(argv[1]);
    }

    if (xs_is_domain_introduced(xs, domid)) {
        printf("Oh yeah, %d is introduced!\n", domid);
    } else {
        printf("Damn, %d is not introduced!\n", domid);
    }

    xs_daemon_close(xs);
}

