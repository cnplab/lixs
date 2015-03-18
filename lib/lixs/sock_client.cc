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


unsigned int lixs::sock_client::next_id = 0;

/* FIXME: What is the correct domid when running in a stub domain? */
lixs::sock_client::sock_client(std::function<void(sock_client*)> dead_cb,
        xenstore& xs, event_mgr& emgr, iomux& io, int fd)
    : client(0, get_id(), xs, emgr, io, fd), emgr(emgr), dead_cb(dead_cb)
{
    std::string path = "/local/domain/0/";

    memcpy(msg.abs_path, path.c_str(), path.length());
    msg.body = msg.abs_path + path.length();
}

lixs::sock_client::~sock_client()
{
}

void lixs::sock_client::conn_dead(void)
{
    emgr.enqueue_event(std::bind(dead_cb, this));
}

std::string lixs::sock_client::get_id(void)
{
    return "S" + std::to_string(next_id++);
}

