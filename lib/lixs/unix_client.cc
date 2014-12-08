#include <lixs/unix_client.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


lixs::unix_client::unix_client(xenstore& xs, event_mgr& emgr, int fd)
    : client(xs, emgr, fd)
{
#ifdef DEBUG
    asprintf(&cid, "C%d", fd);
    printf("%4s = new conn\n", cid);
#endif

    std::string path = "/local/domain/0/";

    memcpy(abs_path, path.c_str(), path.length());
    body = abs_path + path.length();
}

lixs::unix_client::~unix_client()
{
#ifdef DEBUG
    printf("%4s = closed conn\n", cid);
    free(cid);
#endif
}

void lixs::unix_client::create(xenstore& xs, event_mgr& emgr, int fd)
{
    new unix_client(xs, emgr, fd);
}

