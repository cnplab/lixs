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

#ifndef __LIXS_DOMAIN_MGR_HH__
#define __LIXS_DOMAIN_MGR_HH__

#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/log/logger.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <map>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class domain;

class domain_mgr {
public:
    typedef std::map<domid_t, domain*>::iterator iterator;

public:
    domain_mgr(xenstore& xs, event_mgr& emgr, iomux& io, log::logger& log);
    ~domain_mgr();

    int create(domid_t domid, evtchn_port_t port, unsigned int mfn);
    int destroy(domid_t domid);
    void exists(domid_t domid, bool& exists);
    iterator begin(void);
    iterator end(void);

private:
    void domain_dead(domid_t domid);

private:
    typedef std::map<domid_t, domain*> domain_map;

private:
    xenstore& xs;
    event_mgr& emgr;
    iomux& io;
    log::logger& log;

    domain_map domains;
};

} /* namespace lixs */

#endif /* __LIXS_DOMAIN_MGR_HH__ */

