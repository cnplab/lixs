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

#ifndef __LIXS_MSTORE_DATABASE_HH__
#define __LIXS_MSTORE_DATABASE_HH__

#include <lixs/log/logger.hh>
#include <lixs/permissions.hh>

#include <map>
#include <memory>
#include <set>
#include <string>


namespace lixs {
namespace mstore {

class entry {
public:
    entry ()
        : write_seq(0), delete_seq(0), write_children_seq(0)
    { }

    /* Data */
    std::string value;
    permission_list perms;

    /* Metadata for tree management */
    std::set<std::string> children;

    /* Metadata for transaction management */
    long int write_seq;
    long int delete_seq;
    long int write_children_seq;
};

class tentry {
public:
    tentry ()
        : init_seq(0), init_valid(false), read_seq(0), write_seq(0), delete_seq(0),
        read_children_seq(0)
    { }

    /* Data */
    std::string value;
    permission_list perms;

    /* Metadata for tree management */
    std::set<std::string> children;
    std::set<std::string> children_add;
    std::set<std::string> children_rem;

    /* Metadata for transaction management */
    long int init_seq;
    bool init_valid;

    long int read_seq;
    long int write_seq;
    long int delete_seq;

    long int read_children_seq;
};

typedef std::map<unsigned int, tentry> tentry_map;


class record {
public:
    record()
        : next_seq(1)
    { }

    entry e;
    tentry_map te;

    long int next_seq;
};

typedef std::map<std::string, record> database;


class db_access {
public:
    db_access(const std::shared_ptr<database>& db, const std::shared_ptr<log::logger>& log)
        : db(db), log(log)
    { }

    virtual int create(cid_t cid, const std::string& path, bool& created) = 0;
    virtual int read(cid_t cid, const std::string& path, std::string& val) = 0;
    virtual int update(cid_t cid, const std::string& path, const std::string& val) = 0;
    virtual int del(cid_t cid, const std::string& path) = 0;

    virtual int get_children(cid_t cid, const std::string& path, std::set<std::string>& resp) = 0;

    virtual int get_perms(cid_t cid, const std::string& path, permission_list& perms) = 0;
    virtual int set_perms(cid_t cid, const std::string& path, const permission_list& perms) = 0;

protected:
    std::shared_ptr<database> db;

    std::shared_ptr<log::logger> log;
};


bool has_read_access(cid_t cid, const permission_list& perms);
bool has_write_access(cid_t cid, const permission_list& perms);

} /* namespace mstore */
} /* namespace lixs */

#endif /* __LIXS_MSTORE_DATABASE_HH__ */

