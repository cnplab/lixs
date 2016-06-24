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

#ifndef __LIXS_WATCH_MGR_HH__
#define __LIXS_WATCH_MGR_HH__

#include <lixs/event_mgr.hh>
#include <lixs/watch.hh>

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>


namespace lixs {

class watch_mgr {
public:
    watch_mgr(event_mgr& emgr);
    ~watch_mgr();

    void add(watch_cb& cb);
    void del(watch_cb& cb);

    void fire(unsigned int tid, const std::string& path);
    void fire_parents(unsigned int tid, const std::string& path);
    void fire_children(unsigned int tid, const std::string& path);

    void fire_transaction(unsigned int tid);
    void abort_transaction(unsigned int tid);

private:
    typedef std::set<watch_cb*> watch_set;
    struct record {
        watch_set path;
        watch_set children;
    };

    typedef std::map<std::string, record> database;

    typedef std::list<std::function<void(void)> > fire_list;
    typedef std::map<unsigned int, fire_list> transaction_database;

private:
    void callback(const std::string& key, watch_cb* cb, const std::string& path);

    void _fire(const std::string& path, const std::string& fire_path);
    void _tfire(unsigned int tid, const std::string& path, const std::string& fire_path);
    void _fire_parents(const std::string& path, const std::string& fire_path);
    void _tfire_parents(unsigned int tid, const std::string& path, const std::string& fire_path);
    void _fire_children(const std::string& path);
    void _tfire_children(unsigned int tid, const std::string& path);
    void register_with_parents(const std::string& path, watch_cb& cb);
    void unregister_from_parents(const std::string& path, watch_cb& cb);

private:
    event_mgr& emgr;

    database db;
    transaction_database tdb;
};

} /* namespace lixs */

#endif /* __LIXS_WATCH_MGR_HH__ */

