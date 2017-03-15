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

#include <lixs/mstore/store.hh>

#include <memory>


lixs::mstore::store::store(const std::shared_ptr<log::logger>& log)
    : db(new database()), access(db, log), next_tid(1), log(log)
{
}

lixs::mstore::store::~store(void)
{
}

void lixs::mstore::store::branch(unsigned int& tid)
{
    tid = next_tid++;
    trans.insert({tid, transaction(tid, db, log)});
}

int lixs::mstore::store::merge(unsigned int tid, bool& success)
{
    transaction_db::iterator it;

    it = trans.find(tid);
    if (it != trans.end()) {
        it->second.merge(success);
        trans.erase(it);

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::store::abort(unsigned int tid)
{
    transaction_db::iterator it;

    it = trans.find(tid);
    if (it != trans.end()) {
        it->second.abort();
        trans.erase(it);

        return 0;
    } else {
        return ENOENT;
    }
}

int lixs::mstore::store::create(cid_t cid, unsigned int tid, std::string path, bool& created)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.create(cid, path, created);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.create(cid, path, created);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::read(cid_t cid, unsigned int tid, std::string path, std::string& val)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.read(cid, path, val);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.read(cid, path, val);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::update(cid_t cid, unsigned int tid, std::string path, std::string val)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.update(cid, path, val);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.update(cid, path, val);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::del(cid_t cid, unsigned int tid, std::string path)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.del(cid, path);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.del(cid, path);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::get_children(cid_t cid, unsigned int tid, std::string path,
        std::set<std::string>& resp)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.get_children(cid, path, resp);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.get_children(cid, path, resp);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::get_perms(cid_t cid, unsigned int tid,
        std::string path, permission_list& perms)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.get_perms(cid, path, perms);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.get_perms(cid, path, perms);
        } else {
            return EINVAL;
        }
    }
}

int lixs::mstore::store::set_perms(cid_t cid, unsigned int tid,
        std::string path, const permission_list& perms)
{
    if (path.back() == '/') {
        path.pop_back();
    }

    if (tid == 0) {
        return access.set_perms(cid, path, perms);
    } else {
        transaction_db::iterator it;

        it = trans.find(tid);
        if (it != trans.end()) {
            return it->second.set_perms(cid, path, perms);
        } else {
            return EINVAL;
        }
    }
}

