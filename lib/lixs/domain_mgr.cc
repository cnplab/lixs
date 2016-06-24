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

#include <lixs/domain.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/log/logger.hh>

#include <cerrno>
#include <utility>

extern "C" {
#include <xenctrl.h>
}


lixs::domain_mgr::domain_mgr(xenstore& xs, event_mgr& emgr, iomux& io, log::logger& log)
    : xs(xs), emgr(emgr), io(io), log(log)
{
}

lixs::domain_mgr::~domain_mgr()
{
    for (auto& e : domains) {
        delete e.second;
    }
    domains.clear();
}

int lixs::domain_mgr::create(domid_t domid, evtchn_port_t port, unsigned int mfn)
{
    domain* dom;
    domain_map::iterator it;

    it = domains.find(domid);
    if (it == domains.end()) {
        std::function<void(void)> cb = std::bind(&domain_mgr::domain_dead, this, domid);

        try {
            dom = new domain(cb, xs, *this, emgr, io, log, domid, port, mfn);
        } catch (ring_conn_error& e) {
            log::LOG<log::level::ERROR>::logf(log, "[Domain %d] %s", domid, e.what());
            return ECANCELED;
        } catch (foreign_ring_mapper_error& e) {
            log::LOG<log::level::ERROR>::logf(log, "[Domain %d] %s", domid, e.what());
            return ECANCELED;
        }

        /* TODO: Consider using emplace when moving to gcc 4.8 is acceptable */
        domains.insert({domid, dom});

        return 0;
    } else {
        return EEXIST;
    }
}

int lixs::domain_mgr::destroy(domid_t domid)
{
    domain_map::iterator it;

    it = domains.find(domid);
    if (it != domains.end()) {
        delete it->second;
        domains.erase(it);

        return 0;
    } else {
        return ENOENT;
    }
}

void lixs::domain_mgr::exists(domid_t domid, bool& exists)
{
    exists = (domains.find(domid) != domains.end());
}

lixs::domain_mgr::iterator lixs::domain_mgr::begin()
{
    return domains.begin();
}

lixs::domain_mgr::iterator lixs::domain_mgr::end()
{
    return domains.end();
}

void lixs::domain_mgr::domain_dead(domid_t domid)
{
    domain_map::iterator it;

    it = domains.find(domid);
    if (it != domains.end()) {
        delete it->second;
        domains.erase(it);
    }
}

