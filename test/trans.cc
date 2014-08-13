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

    unsigned int len;
    char* str;


    xs = xs_daemon_open();

    xs_write(xs, XBT_NULL, "/test", "1", 1);

    th = xs_transaction_start(xs);

    str = (char*) xs_read(xs, th, "/test", &len);
    printf("%s\n", str);
    if (str) {
        free(str);
    }

    xs_write(xs, th, "/test", "5", 1);

    str = (char*) xs_read(xs, th, "/test", &len);
    printf("%s\n", str);
    if (str) {
        free(str);
    }

    /* This will make the transaction abort since the entry was read */
    xs_write(xs, XBT_NULL, "/test", "3", 1);

    xs_transaction_end(xs, th, 0);

    xs_daemon_close(xs);
}

int main(int argc, char** argv)
{
    do_transaction();
}

