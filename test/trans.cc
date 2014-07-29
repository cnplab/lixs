#include <cstdlib>
#include <cstdio>
#include <cstddef>

extern "C" {
#include <xenstore.h>
}


static void do_transaction()
{
    xs_transaction_t th;
    struct xs_handle *xs;

    xs = xs_daemon_open();

    xs_write(xs, XBT_NULL, "/test", "1", 1);

    th = xs_transaction_start(xs);

    xs_write(xs, XBT_NULL, "/test", "3", 1);

    xs_write(xs, th, "/test", "2", 1);

    xs_transaction_end(xs, th, 0);

    xs_daemon_close(xs);
}

int main(int argc, char** argv)
{
    do_transaction();
}

