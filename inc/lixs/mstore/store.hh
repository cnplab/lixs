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

#ifndef __LIXS_MSTORE_STORE_HH__
#define __LIXS_MSTORE_STORE_HH__

#include <lixs/log/logger.hh>
#include <lixs/store.hh>
#include <lixs/mstore/database.hh>
#include <lixs/mstore/simple_access.hh>
#include <lixs/mstore/transaction.hh>

#include <map>
#include <memory>
#include <set>
#include <string>


namespace lixs {
namespace mstore {

class store : public lixs::store {
public:
    store(const std::shared_ptr<log::logger>& log);
    ~store();

    void branch(unsigned int& tid);
    int merge(unsigned int tid, bool& success);
    int abort(unsigned int tid);

    int create(cid_t cid, unsigned int tid,
            std::string path, bool& created);
    int read(cid_t cid, unsigned int tid,
            std::string key, std::string& val);
    int update(cid_t cid, unsigned int tid,
            std::string key, std::string val);
    int del(cid_t cid, unsigned int tid,
            std::string key);

    int get_children(cid_t cid, unsigned int tid,
            std::string key, std::set<std::string>& resp);

    int get_perms(cid_t cid, unsigned int tid,
            std::string path, permission_list& perms);
    int set_perms(cid_t cid, unsigned int tid,
            std::string path, const permission_list& perms);

private:
    typedef std::map<unsigned int, transaction> transaction_db;


    std::shared_ptr<database> db;

    simple_access access;

    unsigned int next_tid;
    transaction_db trans;

    std::shared_ptr<log::logger> log;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_STORE_HH__ */

