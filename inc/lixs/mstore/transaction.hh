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

#ifndef __LIXS_MSTORE_TRANSACTION_HH__
#define __LIXS_MSTORE_TRANSACTION_HH__

#include <lixs/log/logger.hh>
#include <lixs/mstore/database.hh>

#include <set>
#include <string>


namespace lixs {
namespace mstore {

class transaction : public db_access {
public:
    transaction(unsigned int id, database& db, log::logger& log);

    void abort();
    void merge(bool& success);

    int create(cid_t cid, const std::string& path, bool& created);
    int read(cid_t cid, const std::string& path, std::string& val);
    int update(cid_t cid, const std::string& path, const std::string& val);
    int del(cid_t cid, const std::string& path);

    int get_children(cid_t cid, const std::string& path, std::set<std::string>& resp);

    int get_perms(cid_t cid, const std::string& path, permission_list& perms);
    int set_perms(cid_t cid, const std::string& path, const permission_list& perms);

private:
    bool can_merge();
    void do_merge();

    void register_with_parent(const std::string& path);
    void unregister_from_parent(const std::string& path);
    void ensure_branch(cid_t cid, const std::string& path);
    void delete_branch(const std::string& path, tentry& te);
    void get_parent_perms(const std::string& path, permission_list& perms);
    tentry& get_tentry(const std::string& path, record& rec);
    void fetch_tentry_data(tentry& te, record& rec);
    void fetch_tentry_children(tentry& te, record& rec);

    unsigned int id;
    std::set<std::string> records;
};

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_TRANSACTION_HH__ */

