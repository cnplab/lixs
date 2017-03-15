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

#include <lixs/xenstore.hh>

#include <cerrno>
#include <memory>
#include <set>
#include <string>


lixs::xenstore::xenstore(const std::shared_ptr<store>& st,
        const std::shared_ptr<event_mgr>& emgr,
        const std::shared_ptr<iomux>& io)
    : st(st), wmgr(*emgr)
{
    bool created;

    st->create(0, 0, "/", created);
}

lixs::xenstore::~xenstore()
{
}

int lixs::xenstore::store_read(cid_t cid, unsigned int tid,
        const std::string& path, std::string& val)
{
    return st->read(cid, tid, path, val);
}

int lixs::xenstore::store_write(cid_t cid, unsigned int tid,
        const std::string& path, const std::string& val)
{
    int ret;

    ret = st->update(cid, tid, path, val);
    if (ret == 0) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
    }

    return ret;
}

int lixs::xenstore::store_mkdir(cid_t cid, unsigned int tid,
        const std::string& path)
{
    int ret;
    bool created;

    ret = st->create(cid, tid, path, created);
    if (ret == 0 && created) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
    }

    return ret;
}

int lixs::xenstore::store_rm(cid_t cid, unsigned int tid,
        const std::string& path)
{
    int ret;

    ret = st->del(cid, tid, path);
    if (ret == 0) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
        wmgr.fire_children(tid, path);
    }

    return ret;
}

int lixs::xenstore::store_dir(cid_t cid, unsigned int tid,
        const std::string& path, std::set<std::string>& res)
{
    return st->get_children(cid, tid, path, res);
}

int lixs::xenstore::store_get_perms(cid_t cid, unsigned int tid,
        const std::string& path, permission_list& perms)
{
    return st->get_perms(cid, tid, path, perms);
}

int lixs::xenstore::store_set_perms(cid_t cid, unsigned int tid,
        const std::string& path, const permission_list& perms)
{
    int ret;

    ret = st->set_perms(cid, tid, path, perms);
    if (ret == 0) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
    }

    return ret;
}

int lixs::xenstore::transaction_start(cid_t cid, unsigned int* tid)
{
    st->branch(*tid);

    return 0;
}

int lixs::xenstore::transaction_end(cid_t cid, unsigned int tid, bool commit)
{
    int ret;
    bool success;

    if (commit) {
        ret = st->merge(tid, success);

        if (ret == 0) {
            if (success) {
                wmgr.fire_transaction(tid);
            } else {
                wmgr.abort_transaction(tid);
            }

            return success ? 0 : EAGAIN;
        } else {
            return ret;
        }
    } else {
        ret = st->abort(tid);

        if (ret == 0) {
            wmgr.abort_transaction(tid);

            return 0;
        } else {
            return ret;
        }
    }
}

void lixs::xenstore::watch_add(watch_cb& cb)
{
    wmgr.add(cb);
}

void lixs::xenstore::watch_del(watch_cb& cb)
{
    wmgr.del(cb);
}

void lixs::xenstore::domain_path(domid_t domid, std::string& path)
{
    char numstr[35];
    sprintf(numstr, "/local/domain/%d", domid);

    path = std::string(numstr);
}

void lixs::xenstore::domain_introduce(domid_t domid)
{
    wmgr.fire(0, "@introduceDomain");
}

void lixs::xenstore::domain_release(domid_t domid)
{
    wmgr.fire(0, "@releaseDomain");
}

