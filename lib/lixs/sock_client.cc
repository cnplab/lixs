#include <lixs/domain_mgr.hh>
#include <lixs/log/logger.hh>
#include <lixs/sock_client.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


/* FIXME: What is the correct domid when running in a stub domain? */
lixs::sock_client::sock_client(long unsigned int id, std::function<void(void)> dead_cb,
        xenstore& xs, domain_mgr& dmgr, event_mgr& emgr, iomux& io, log::logger& log, int fd)
    : client(get_id(id), log, 0, xs, dmgr, io, fd), id(id), emgr(emgr), dead_cb(dead_cb)
{
}

lixs::sock_client::~sock_client()
{
}

void lixs::sock_client::conn_dead(void)
{
    emgr.enqueue_event(dead_cb);
}

std::string lixs::sock_client::get_id(long unsigned int id)
{
    return "S" + std::to_string(id);
}

