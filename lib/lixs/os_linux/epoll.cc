/*
 * LiXS: Lightweight XenStore
 *
 * Authors: Filipe Manco <filipe.manco@neclab.eu>
 *
 *
 * Copyright (c) 2016, NEC Europe Ltd., NEC Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include <lixs/os_linux/epoll.hh>

#include <cstddef>
#include <sys/epoll.h>


lixs::os_linux::epoll::epoll(event_mgr& emgr)
    : iomux(emgr), epfd(epoll_create(0x7E57))
{
    emgr.enqueue_event(std::bind(&epoll::handle, this));
}

lixs::os_linux::epoll::~epoll()
{
}

void lixs::os_linux::epoll::add(int fd, bool read, bool write, io_cb cb)
{
    std::pair<cb_map::iterator, bool> res = callbacks.insert({fd, cb});

    if (!res.second) {
        return;
    }

    struct epoll_event event = {
        get_events(read, write),
        { static_cast<void*>(&(res.first->second)) }
    };

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

void lixs::os_linux::epoll::set(int fd, bool read, bool write)
{
    cb_map::iterator it = callbacks.find(fd);

    if (it == callbacks.end()) {
        return;
    }

    struct epoll_event event = {
        get_events(read, write),
        { static_cast<void*>(&(it->second)) }
    };

    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}

void lixs::os_linux::epoll::rem(int fd)
{
    cb_map::size_type res = callbacks.erase(fd);

    if (res == 0) {
        return;
    }

    /* Passing event == NULL requires linux > 2.6.9, see BUGS */
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

void lixs::os_linux::epoll::handle(void)
{
    int n_events;

    n_events = epoll_wait(epfd, epev, epoll_max_events, timeout);

    for (int i = 0; i < n_events; i++) {
        emgr.enqueue_event(std::bind(*static_cast<io_cb*>(epev[i].data.ptr),
                is_read(epev[i].events), is_write(epev[i].events), is_error(epev[i].events)));
    }

    emgr.enqueue_event(std::bind(&epoll::handle, this));
}

uint32_t inline lixs::os_linux::epoll::get_events(bool read, bool write)
{
    return (read ? EPOLLIN : 0) | (write ? EPOLLOUT : 0);
}

bool inline lixs::os_linux::epoll::is_read(const uint32_t ev)
{
    return (ev & EPOLLIN) != 0;
}

bool inline lixs::os_linux::epoll::is_write(const uint32_t ev)
{
    return (ev & EPOLLOUT) != 0;
}

bool inline lixs::os_linux::epoll::is_error(const uint32_t ev)
{
    return (ev & (EPOLLHUP | EPOLLERR)) != 0;
}

