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

#include <lixs/xs_proto_v1/xs_proto.hh>

#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>


namespace lixs {
namespace xs_proto_v1 {

xs_proto_base::xs_proto_base(domid_t domid, xenstore& xs, domain_mgr& dmgr, log::logger& log)
    : domid(domid), dom_path(get_dom_path(domid, xs)),
    rx_msg(dom_path), tx_msg(dom_path), xs(xs), dmgr(dmgr), log(log)
{
}

xs_proto_base::~xs_proto_base()
{
}

void xs_proto_base::handle_rx(void)
{
    switch (rx_msg.hdr.type) {
        case XS_DIRECTORY:
            op_directory();
        break;

        case XS_READ:
            op_read();
        break;

        case XS_WRITE:
            op_write();
        break;

        case XS_MKDIR:
            op_mkdir();
        break;

        case XS_RM:
            op_rm();
        break;

        case XS_GET_PERMS:
            op_get_perms();
        break;

        case XS_SET_PERMS:
            op_set_perms();
        break;

        case XS_DEBUG:
            op_unimplemented();
        break;

        case XS_WATCH:
            op_watch();
        break;

        case XS_UNWATCH:
            op_unwatch();
        break;

        case XS_TRANSACTION_START:
            op_transaction_start();
        break;

        case XS_TRANSACTION_END:
            op_transaction_end();
        break;

        case XS_INTRODUCE:
            op_introduce();
        break;

        case XS_IS_DOMAIN_INTRODUCED:
            op_is_domain_introduced();
        break;

        case XS_RELEASE:
            op_release();
        break;

        case XS_GET_DOMAIN_PATH:
            op_get_domain_path();
        break;

        case XS_RESUME:
            op_unimplemented();
        break;

        case XS_SET_TARGET:
            op_unimplemented();
        break;

        case XS_RESET_WATCHES:
            op_unimplemented();
        break;

        case XS_RESTRICT:
            op_unimplemented();
        break;

        default:
            op_unimplemented();
        break;
    }

    /* As result of the message processing a response might have been
     * generated. Trigger tx here.
     */
    process_tx();
}


void xs_proto_base::op_directory(void)
{
    int ret;
    std::set<std::string> result;

    ret = xs.store_dir(domid, rx_msg.hdr.tx_id, get_path(), result);

    if (ret == 0) {
        tx_queue.push_back({XS_DIRECTORY, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {result.begin(), result.end()}, !result.empty()});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_read(void)
{
    int ret;
    std::string result;

    ret = xs.store_read(domid, rx_msg.hdr.tx_id, get_path(), result);

    if (ret == 0) {
        tx_queue.push_back({XS_READ, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {result}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_write(void)
{
    int ret;

    ret = xs.store_write(domid, rx_msg.hdr.tx_id, get_path(), get_arg2());

    if (ret == 0) {
        tx_queue.push_back({XS_WRITE, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_mkdir(void)
{
    int ret;

    ret = xs.store_mkdir(domid, rx_msg.hdr.tx_id, get_path());

    if (ret == 0) {
        tx_queue.push_back({XS_MKDIR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_rm(void)
{
    int ret;

    ret = xs.store_rm(domid, rx_msg.hdr.tx_id, get_path());

    if (ret == 0) {
        tx_queue.push_back({XS_RM, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}


void xs_proto_base::op_get_perms(void)
{
    int ret;
    permission_list result;

    std::string perm_str;
    std::list<std::string> result_str;

    ret = xs.store_get_perms(domid, rx_msg.hdr.tx_id, get_path(), result);

    if (ret == 0) {
        for (auto& r : result) {
            perm2str(r, perm_str);
            result_str.push_back(perm_str);
        }

        tx_queue.push_back({XS_GET_PERMS, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {result_str}, true});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_set_perms(void)
{
    int ret;
    char* arg;
    char* path;
    bool perms_ok;
    permission perm;
    permission_list perms;

    path = get_path();

    arg = path;
    perms_ok = true;
    while ((arg = get_next_arg(arg)) && (perms_ok = str2perm(arg, perm))) {
        perms.push_back(perm);
    }

    if (!perms_ok) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    ret = xs.store_set_perms(domid, rx_msg.hdr.tx_id, path, perms);

    if (ret == 0) {
        tx_queue.push_back({XS_SET_PERMS, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_watch(void)
{
    char* path;
    char* token;
    bool relative;
    watch_map::iterator it;

    path = get_path();
    token = get_arg2();
    relative = (path != rx_msg.body);

    it = watches.find({path, token});
    if (it == watches.end()) {
        it = watches.insert({{path, token}, {*this, path, token, relative}}).first;

        xs.watch_add(it->second);

        tx_queue.push_back({XS_WATCH, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(0)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EEXIST)}, false});
    }
}

void xs_proto_base::op_unwatch(void)
{
    char* path;
    char* token;
    watch_map::iterator it;

    path = get_path();
    token = get_arg2();

    it = watches.find({path, token});
    if (it == watches.end()) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ENOENT)}, false});
    } else {
        xs.watch_del(it->second);

        watches.erase(it);

        tx_queue.push_back({XS_UNWATCH, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(0)}, false});
    }
}

void xs_proto_base::op_transaction_start(void)
{
    unsigned int tid;

    xs.transaction_start(domid, &tid);

    tx_queue.push_back({XS_TRANSACTION_START, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
            {std::to_string(tid)}, true});
}

void xs_proto_base::op_transaction_end(void)
{
    int ret;
    char* arg;
    bool commit;

    arg = get_arg1();

    if (strcmp(arg, "T") == 0) {
        commit = true;
    } else if (strcmp(arg, "F") == 0) {
        commit = false;
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    ret = xs.transaction_end(domid, rx_msg.hdr.tx_id, commit);

    if (ret == 0) {
        tx_queue.push_back({XS_TRANSACTION_END, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_introduce(void)
{
    int ret;
    domid_t domid;
    unsigned int mfn;
    evtchn_port_t port;

    ret = get_int(get_arg1(), domid);
    if (ret != 0) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    ret = get_int(get_arg2(), mfn);
    if (ret != 0) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    ret = get_int(get_arg3(), port);
    if (ret != 0) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    ret = dmgr.create(domid, port, mfn);

    if (ret == 0) {
        xs.domain_introduce(domid);

        tx_queue.push_back({XS_INTRODUCE, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_release(void)
{
    int ret;
    domid_t domid;

    ret = get_int(get_arg1(), domid);
    if (ret != 0) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    ret = dmgr.destroy(domid);

    if (ret == 0) {
        xs.domain_release(domid);

        tx_queue.push_back({XS_RELEASE, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    } else {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(ret)}, false});
    }
}

void xs_proto_base::op_is_domain_introduced(void)
{
    int ret;
    bool exists;
    domid_t domid;
    std::string exists_str;

    ret = get_int(get_arg1(), domid);
    if (ret != 0) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    dmgr.exists(domid, exists);

    exists_str = exists ? "T" : "F";

    tx_queue.push_back({XS_IS_DOMAIN_INTRODUCED, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
            {exists_str}, false});
}

void xs_proto_base::op_get_domain_path(void)
{
    int ret;
    domid_t domid;

    ret = get_int(get_arg1(), domid);
    if (ret != 0) {
        tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
                {err2str(EINVAL)}, false});
        return;
    }

    std::string path;
    xs.domain_path(domid, path);

    tx_queue.push_back({XS_GET_DOMAIN_PATH, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
            {path}, false});
}

void xs_proto_base::op_unimplemented(void)
{
    tx_queue.push_back({XS_ERROR, rx_msg.hdr.req_id, rx_msg.hdr.tx_id,
            {err2str(ENOSYS)}, false});
}

bool xs_proto_base::prepare_tx(void)
{
    if (tx_queue.empty()) {
        return false;
    }

    message msg = tx_queue.front();
    tx_queue.pop_front();

    build_hdr(msg.type, msg.req_id, msg.tx_id);
    if (!build_body(msg.body, msg.terminator)) {
        build_hdr(XS_ERROR, msg.req_id, msg.tx_id);
        build_body(err2str(E2BIG), false);
    }

    return true;
}

void xs_proto_base::perm2str(const permission& perm, std::string& str)
{
    if (perm.read && perm.write) {
        str = "b";
    } else if (perm.read) {
        str = "r";
    } else if (perm.write) {
        str = "w";
    } else {
        str = "n";
    }

    str += std::to_string(perm.cid);
}

bool xs_proto_base::str2perm(const std::string& str, permission& perm)
{
    domid_t domid;

    if (str.length() < 2) {
        return false;
    }

    switch (str[0]) {
        case 'b':
            perm.read = true;
            perm.write = true;
            break;

        case 'r':
            perm.read = true;
            perm.write = false;
            break;

        case 'w':
            perm.read = false;
            perm.write = true;
            break;

        case 'n':
            perm.read = false;
            perm.write = false;
            break;

        default:
            return false;
    }

    if (get_int(str.substr(1).c_str(), domid)) {
        return false;
    }

    perm.cid = static_cast<cid_t>(domid);

    return true;
}

std::string xs_proto_base::err2str(int err)
{
    unsigned int i;
    const char* err_str;

    if (err == 0) {
        return "OK";
    }

    for (i = 0; i < (sizeof(xsd_errors) / sizeof(xsd_errors[0])); i++) {
        if (err == xsd_errors[i].errnum) {
            err_str = xsd_errors[i].errstring;
            break;
        }
    }

    /* NOTE: If err is not in xsd_errors just fall back to EINVAL.
     */
    if (i == (sizeof(xsd_errors) / sizeof(xsd_errors[0]))) {
        err_str = (char*) "EINVAL";
    }

    return err_str;
}

std::string xs_proto_base::path2rel(std::string path)
{
    return path.substr(dom_path.length());
}

char* xs_proto_base::get_arg1(void)
{
    return rx_msg.body;
}

char* xs_proto_base::get_arg2(void)
{
    return rx_msg.body + strlen(rx_msg.body) + 1;
}

char* xs_proto_base::get_arg3(void)
{
    char* arg2 = get_arg2();

    return arg2 + strlen(arg2) + 1;
}

char* xs_proto_base::get_next_arg(char* curr)
{
    char* next = curr + strlen(curr) + 1;

    return ((uint32_t) (next - rx_msg.body)) >= rx_msg.hdr.len ? NULL : next;
}

char* xs_proto_base::get_path(void)
{
    if (rx_msg.body[0] == '/' || rx_msg.body[0] == '@') {
        return rx_msg.body;
    } else {
        return rx_msg.abs_path;
    }
}

template<typename int_t>
int xs_proto_base::get_int(const char* arg, int_t& number)
{
    long long n;

    try {
        n = std::stoll(arg);
    } catch(std::invalid_argument& e) {
        return EINVAL;
    } catch(std::out_of_range& e) {
        return EINVAL;
    }

    if (n < std::numeric_limits<int_t>::min() || n > std::numeric_limits<int_t>::max()) {
        return EINVAL;
    }

    number = static_cast<int_t>(n);

    return 0;
}

void xs_proto_base::build_hdr(uint32_t type, uint32_t req_id, uint32_t tx_id)
{
    tx_msg.hdr.type = type;
    tx_msg.hdr.req_id = req_id;
    tx_msg.hdr.tx_id = tx_id;
    tx_msg.hdr.len = 0;
}

bool xs_proto_base::build_body(std::string elem, bool terminator)
{
    if (elem.length() == 0) {
        tx_msg.body[0] = '\0';
        tx_msg.hdr.len = terminator ? 1 : 0;
        return true;
    }

    if (elem.length() > XENSTORE_PAYLOAD_MAX) {
        /* FIXME: should log some error */
        return false;
    }

    memcpy(tx_msg.body, elem.c_str(), elem.length());
    tx_msg.body[elem.length()] = '\0';
    tx_msg.hdr.len = elem.length();

    if (terminator) {
        tx_msg.hdr.len += 1;
    }

    return true;
}

bool xs_proto_base::build_body(std::list<std::string> elems, bool terminator)
{
    char* body;
    uint32_t length;

    if (elems.empty()) {
        tx_msg.body[0] = '\0';
        tx_msg.hdr.len = terminator ? 1 : 0;
        return true;
    }

    /* Calculate total message length for checks */
    length = 0;
    for (auto& e : elems) {
        /* String plus null separators */
        length += e.length() + 1;
    }

    /* If not null terminated don't count the last \0 */
    if (!terminator) {
        length -= 1;
    }

    if (length > XENSTORE_PAYLOAD_MAX) {
        /* FIXME: should log some error */
        return false;
    }

    body = tx_msg.body;
    for (auto& e : elems) {
        memcpy(body, e.c_str(), e.length());
        body[e.length()] = '\0';
        body += e.length() + 1;
    }
    tx_msg.hdr.len = length;

    return true;
}

std::string xs_proto_base::get_dom_path(domid_t domid, xenstore& xs)
{
    std::string path;

    xs.domain_path(domid, path);

    return path;
}

} /* namespace xs_proto_v1 */
} /* namespace lixs */

