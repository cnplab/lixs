#include <lixs/client.hh>
#include <lixs/domain_mgr.hh>
#include <lixs/event_mgr.hh>
#include <lixs/watch.hh>
#include <lixs/xenstore.hh>

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <utility>

extern "C" {
#include <xen/io/xs_wire.h>
}


lixs::client_base::client_base(domid_t domid, const std::string& id,
        xenstore& xs, domain_mgr& dmgr, event_mgr& emgr)
    : xs(xs), state(p_rx), domid(domid), id(id), dmgr(dmgr), emgr(emgr)
{
#ifdef DEBUG
    printf("%4s = new conn\n", id.c_str());
#endif

    emgr.enqueue_event(std::bind(&client_base::process, this));
}

lixs::client_base::~client_base()
{
#ifdef DEBUG
    printf("%4s = closed conn\n", id.c_str());
#endif
}

void lixs::client_base::watch_cb_k::operator()(const std::string& path)
{
    std::string fire_path = path.substr(relative ? client.msg.body - client.msg.abs_path : 0);

    client.watch_fired(fire_path, token);
}

void lixs::client_base::handle_msg(void)
{
    /* FIXME: This is a quick fix, need to analyse this better */
    /* Ensure the body is null terminated */
    msg.body[msg.hdr.len] = '\0';

#ifdef DEBUG
    print_msg((char*)"<");
#endif

    switch (msg.hdr.type) {
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
            op_debug();
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
            op_resume();
        break;

        case XS_SET_TARGET:
            op_set_target();
        break;

        case XS_RESET_WATCHES:
            op_reset_watches();
        break;

        case XS_RESTRICT:
            op_restrict();
        break;

        default:
            build_err(ENOSYS);
        break;
    }

#ifdef DEBUG
    print_msg((char*)">");
#endif
}

void lixs::client_base::prepare_watch(const std::pair<std::string, std::string>& watch)
{
    build_watch(watch.first.c_str(), watch.second.c_str());

#ifdef DEBUG
    print_msg((char*)">");
#endif
}

void lixs::client_base::op_read(void)
{
    int ret;
    std::string res;

    ret = xs.store_read(domid, msg.hdr.tx_id, get_path(), res);

    if (ret == 0) {
        if (!build_resp(res.c_str())) {
            build_err(E2BIG);
        }
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_write(void)
{
    int ret;

    ret = xs.store_write(domid, msg.hdr.tx_id, get_path(), get_arg2());

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_mkdir(void)
{
    int ret;

    ret = xs.store_mkdir(domid, msg.hdr.tx_id, get_path());

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_rm(void)
{
    int ret;

    ret = xs.store_rm(domid, msg.hdr.tx_id, get_path());

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_transaction_start(void)
{
    unsigned int tid;

    xs.transaction_start(domid, &tid);

    if (!build_resp(std::to_string(tid).c_str()) || !append_sep()) {
        build_err(E2BIG);
    }
}

void lixs::client_base::op_transaction_end(void)
{
    int ret;

    ret = xs.transaction_end(domid, msg.hdr.tx_id, strcmp(get_arg1(), "T") == 0);

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_get_domain_path(void)
{
    domid_t req_domid;
    std::string path;

    try {
        req_domid = std::stoi(get_arg1());
    } catch(std::invalid_argument e) {
        build_err(EINVAL);
        return;
    } catch(std::out_of_range e) {
        build_err(EINVAL);
        return;
    }

    xs.domain_path(req_domid, path);

    if (!build_resp(path.c_str())) {
        build_err(E2BIG);
    }
}

void lixs::client_base::op_get_perms(void)
{
    int ret;
    bool ok;
    std::string str;
    permission_list resp;
    permission_list::iterator it;

    ret = xs.store_get_perms(domid, msg.hdr.tx_id, get_path(), resp);

    if (ret == 0) {
        ok = build_resp("");
        for (it = resp.begin(); it != resp.end() && ok; it++) {
            perm2str(*it, str);
            ok = append_resp(str.c_str()) && append_sep();
        }

        if (!ok) {
            build_err(E2BIG);
        }
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_set_perms(void)
{
    int ret;
    bool ok;
    char* arg;
    char* path;
    permission perm;
    permission_list perms;

    path = get_path();

    ok = true;
    arg = path;
    while ((arg = get_next_arg(arg))) {
        ok = str2perm(arg, perm);

        if (!ok) {
            break;
        }

        perms.push_back(perm);
    }

    if (ok) {
        ret = xs.store_set_perms(domid, msg.hdr.tx_id, path, perms);

        if (ret == 0) {
            build_ack();
        } else {
            build_err(ret);
        }
    } else {
        build_err(EINVAL);
    }
}

void lixs::client_base::op_set_target(void)
{
    /* FIXME: implement */

    build_err(ENOSYS);
}

void lixs::client_base::op_restrict(void)
{
    /* FIXME: implement */

    build_err(ENOSYS);
}

void lixs::client_base::op_directory(void)
{
    int ret;
    bool ok;
    std::set<std::string> resp;
    std::set<std::string>::iterator it;

    ret = xs.store_dir(domid, msg.hdr.tx_id, get_path(), resp);

    if (ret == 0) {
        ok = build_resp("");
        for (it = resp.begin(); it != resp.end() && ok; it++) {
            ok = append_resp((*it).c_str()) && append_sep();
        }

        if (!ok) {
            build_err(E2BIG);
        }
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_watch(void)
{
    char* path;
    watch_map::iterator it;

    path = get_path();

    it = watches.find(path);
    if (it == watches.end()) {
        it = watches.insert(
                std::pair<std::string, watch_cb_k>(
                    path, watch_cb_k(*this, path, get_arg2(), path != msg.body))).first;
        xs.watch_add(it->second);
    }

    build_ack();
}

void lixs::client_base::op_unwatch(void)
{
    char* path;
    watch_map::iterator it;

    path = get_path();

    it = watches.find(path);
    if (it != watches.end()) {
        xs.watch_del(it->second);
        watches.erase(it);
        build_ack();
    } else {
        build_err(ENOENT);
    }
}

void lixs::client_base::op_reset_watches(void)
{
    /* FIXME: implement */

    build_err(ENOSYS);
}

void lixs::client_base::op_introduce(void)
{
    int ret;
    domid_t domid;
    unsigned int mfn;
    evtchn_port_t port;

    domid = atoi(get_arg1());
    mfn = atoi(get_arg2());
    port = atoi(get_arg3());

    ret = dmgr.create(domid, port, mfn);
    if (ret == 0) {
        xs.domain_introduce(domid, port, mfn);
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_release(void)
{
    int ret;
    domid_t domid;

    domid = atoi(get_arg1());

    ret = dmgr.destroy(domid);
    if (ret == 0) {
        xs.domain_release(domid);
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client_base::op_is_domain_introduced(void)
{
    bool exists;

    dmgr.exists(atoi(get_arg1()), exists);

    if (!build_resp(exists ? "T" : "F") || !append_sep()) {
        build_err(E2BIG);
    }
}

void lixs::client_base::op_debug(void)
{
    /* FIXME: implement */

    build_err(ENOSYS);
}

void lixs::client_base::op_resume(void)
{
    /* FIXME: implement */

    build_err(ENOSYS);
}

void lixs::client_base::perm2str(const permission& perm, std::string& str)
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

bool lixs::client_base::str2perm(const std::string& str, permission& perm)
{
    int domid;

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

    try {
        domid = std::stoi(str.substr(1));
    } catch(std::invalid_argument e) {
        return false;
    } catch(std::out_of_range e) {
        return false;
    }

    if (domid < 0) {
        return false;
    }

    perm.cid = (cid_t) domid;

    return true;
}

char* lixs::client_base::get_path(void)
{
    if (msg.body[0] == '/' || msg.body[0] == '@') {
        return msg.body;
    } else {
        return msg.abs_path;
    }
}

char* lixs::client_base::get_arg1(void)
{
    return msg.body;
}

char* lixs::client_base::get_arg2(void)
{
    return msg.body + strlen(msg.body) + 1;
}

char* lixs::client_base::get_arg3(void)
{
    char* arg2 = get_arg2();

    return arg2 + strlen(arg2) + 1;
}

char* lixs::client_base::get_next_arg(char* curr)
{
    char* next = curr + strlen(curr) + 1;

    return (next - msg.body) >= msg.hdr.len ? NULL : next;
}

bool lixs::client_base::build_resp(const char* resp)
{
    int len = strlen(resp);

    if (len > XENSTORE_PAYLOAD_MAX) {
        return false;
    }

    memcpy(msg.body, resp, len);
    msg.hdr.len = len;

    return true;
}

bool lixs::client_base::append_resp(const char* resp)
{
    int len = strlen(resp);

    if (len > XENSTORE_PAYLOAD_MAX) {
        return false;
    }

    memcpy(msg.body + msg.hdr.len, resp, len);
    msg.hdr.len += len;

    return true;
}

bool lixs::client_base::append_sep(void)
{
    int len = msg.hdr.len + 1;

    if (len > XENSTORE_PAYLOAD_MAX) {
        return false;
    }

    msg.body[msg.hdr.len++] = '\0';

    return true;
}

bool lixs::client_base::build_watch(const char* path, const char* token)
{
    int path_len = strlen(path);
    int token_len = strlen(token);
    int total_len = path_len + token_len + 2;

    if (total_len > XENSTORE_PAYLOAD_MAX) {
        return false;
    }

    msg.hdr.type = XS_WATCH_EVENT;
    msg.hdr.req_id = 0;
    msg.hdr.tx_id = 0;

    memcpy(msg.body, path, path_len);
    msg.body[path_len] = '\0';
    memcpy(msg.body + path_len + 1, token, token_len);
    msg.body[path_len + 1 + token_len] = '\0';

    msg.hdr.len = total_len;

    return true;
}

void lixs::client_base::build_err(int err)
{
    const char* resp;

    /* FIXME: What if err is not in xsd_error */
    resp = (char*) "EINVAL";

    for (unsigned int i = 0; i < (sizeof(xsd_errors) / sizeof(xsd_errors[0])); i++) {
        if (err == xsd_errors[i].errnum) {
            resp = xsd_errors[i].errstring;
            break;
        }
    }

    msg.hdr.len = strlen(resp) + 1;
    msg.hdr.type = XS_ERROR;
    memcpy(msg.body, resp, msg.hdr.len);
}

void lixs::client_base::build_ack(void)
{
    msg.hdr.len = 3;
    memcpy(msg.body, "OK", 3);
}

#ifdef DEBUG
void lixs::client_base::print_msg(char* pre)
{
    unsigned int i;
    char c;

    msg.body[msg.hdr.len] = '\0';

    printf("%4s %s { type = %2d, req_id = %d, tx_id = %d, len = %d, msg = ",
            id.c_str(), pre, msg.hdr.type, msg.hdr.req_id, msg.hdr.tx_id, msg.hdr.len);

    c = '"';
    for (i = 0; i < msg.hdr.len; i += strlen(msg.body + i) + 1) {
        printf("%c%s", c, msg.body + i);
        c = ' ';
    }

    printf("%s%s\" }\n", i == 0 ? "\"" : "", i > 0 && i == msg.hdr.len ? " " : "");
}
#endif

