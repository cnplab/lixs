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

#include <lixs/util.hh>
#include <lixs/watch.hh>
#include <lixs/watch_mgr.hh>

#include <memory>


lixs::watch_mgr::watch_mgr(const std::shared_ptr<event_mgr>& emgr)
    : emgr(emgr)
{
}

lixs::watch_mgr::~watch_mgr()
{
}

void lixs::watch_mgr::add(watch_cb& cb)
{
    db[cb.path].path.insert(&cb);

    register_with_parents(cb.path, cb);

    emgr->enqueue_event(std::bind(&watch_mgr::callback, this, cb.path, &cb, cb.path));
}

void lixs::watch_mgr::del(watch_cb& cb)
{
    record& rec = db[cb.path];
    rec.path.erase(&cb);
    if (rec.path.empty() && rec.children.empty()) {
        db.erase(cb.path);
    }

    unregister_from_parents(cb.path, cb);
}

void lixs::watch_mgr::fire(unsigned int tid, const std::string& path)
{
    if (tid == 0) {
        _fire(path, path);
    } else {
        _tfire(tid, path, path);
    }
}

void lixs::watch_mgr::fire_parents(unsigned int tid, const std::string& path)
{
    if (tid == 0) {
        _fire_parents(path, path);
    } else {
        _tfire_parents(tid, path, path);
    }
}

void lixs::watch_mgr::fire_children(unsigned int tid, const std::string& path)
{
    if (tid == 0) {
        _fire_children(path);
    } else {
        _tfire_children(tid, path);
    }
}

void lixs::watch_mgr::fire_transaction(unsigned int tid)
{
    for (auto& t : tdb[tid]) {
        emgr->enqueue_event(t);
    }

    tdb.erase(tid);
}

void lixs::watch_mgr::abort_transaction(unsigned int tid)
{
    tdb.erase(tid);
}

void lixs::watch_mgr::callback(const std::string& key, watch_cb* cb, const std::string& path)
{
    database::iterator it;

    it = db.find(key);
    if (it != db.end()) {
        record& rec = it->second;

        if (rec.path.find(cb) != rec.path.end() || rec.children.find(cb) != rec.children.end()) {
            cb->operator()(path);
        }
    }
}

void lixs::watch_mgr::_fire(const std::string& path, const std::string& fire_path)
{
    for (auto& r : db[path].path) {
        emgr->enqueue_event(std::bind(&watch_mgr::callback, this, path, r, fire_path));
    }
}

void lixs::watch_mgr::_tfire(unsigned int tid, const std::string& path,
        const std::string& fire_path)
{
    fire_list& l = tdb[tid];

    for (auto& p : db[path].path) {
        l.push_back(std::bind(&watch_mgr::callback, this, path, p, fire_path));
    }
}

void lixs::watch_mgr::_fire_parents(const std::string& path, const std::string& fire_path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        _fire(parent, fire_path);
        _fire_parents(parent, fire_path);
    }
}

void lixs::watch_mgr::_tfire_parents(unsigned int tid, const std::string& path,
        const std::string& fire_path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        _tfire(tid, parent, fire_path);
        _tfire_parents(tid, parent, fire_path);
    }
}

void lixs::watch_mgr::_fire_children(const std::string& path)
{
    for (auto& c : db[path].children) {
        emgr->enqueue_event(std::bind(&watch_mgr::callback, this, path, c, c->path));
    }
}

void lixs::watch_mgr::_tfire_children(unsigned int tid, const std::string& path)
{
    fire_list& l = tdb[tid];

    for (auto& c : db[path].children) {
        l.push_back(std::bind(&watch_mgr::callback, this, path, c, c->path));
    }
}

void lixs::watch_mgr::register_with_parents(const std::string& path, watch_cb& cb)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        db[parent].children.insert(&cb);

        register_with_parents(parent, cb);
    }
}

void lixs::watch_mgr::unregister_from_parents(const std::string& path, watch_cb& cb)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        record& rec = db[parent];

        rec.children.erase(&cb);
        if (rec.path.empty() && rec.children.empty()) {
            db.erase(parent);
        }

        unregister_from_parents(parent, cb);
    }
}

