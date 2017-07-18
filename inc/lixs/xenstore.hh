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

#ifndef __LIXS_XENSTORE_HH__
#define __LIXS_XENSTORE_HH__

#include <lixs/event_mgr.hh>
#include <lixs/permissions.hh>
#include <lixs/store.hh>
#include <lixs/watch_mgr.hh>

#include <cerrno>
#include <memory>
#include <string>
#include <set>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {

class xenstore {
public:
    xenstore(const std::shared_ptr<store>& st,
            const std::shared_ptr<event_mgr>& emgr);
    ~xenstore();

    int store_read(cid_t cid, unsigned int tid,
            const std::string& path, std::string& val);
    int store_write(cid_t cid, unsigned int tid,
            const std::string& path, const std::string& val);
    int store_mkdir(cid_t cid, unsigned int tid,
            const std::string& path);
    int store_rm(cid_t cid, unsigned int tid,
            const std::string& path);
    int store_dir(cid_t cid, unsigned int tid,
            const std::string& path, std::set<std::string>& res);
    int store_get_perms(cid_t cid, unsigned int tid,
            const std::string& path, permission_list& perms);
    int store_set_perms(cid_t cid, unsigned int tid,
            const std::string& path, const permission_list& perms);

    int transaction_start(cid_t cid, unsigned int* tid);
    int transaction_end(cid_t cid, unsigned int tid, bool commit);

    void watch_add(watch_cb& cb);
    void watch_del(watch_cb& cb);

    /* FIXME: should domain operations also receive a client id? */
    void domain_path(domid_t domid, std::string& path);
    void domain_introduce(domid_t domid);
    void domain_release(domid_t domid);

private:
    std::shared_ptr<store> st;

    watch_mgr wmgr;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

