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

#ifndef __LIXS_STORE_HH__
#define __LIXS_STORE_HH__

#include <lixs/permissions.hh>

#include <set>
#include <string>


namespace lixs {

class store {
public:
    virtual void branch(unsigned int& tid) = 0;
    virtual int merge(unsigned int tid, bool& success) = 0;
    virtual int abort(unsigned int tid) = 0;

    virtual int create(cid_t cid, unsigned int tid,
            std::string path, bool& created) = 0;
    virtual int read(cid_t cid, unsigned int tid,
            std::string path, std::string& val) = 0;
    virtual int update(cid_t cid, unsigned int tid,
            std::string path, std::string val) = 0;
    virtual int del(cid_t cid, unsigned int tid,
            std::string path) = 0;

    virtual int get_children(cid_t cid, unsigned int tid,
            std::string path, std::set<std::string>& resp) = 0;

    virtual int get_perms(cid_t cid, unsigned int tid,
            std::string path, permission_list& perms) = 0;
    virtual int set_perms(cid_t cid, unsigned int tid,
            std::string path, const permission_list& perms) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_STORE_HH__ */

