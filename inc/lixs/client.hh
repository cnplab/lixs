#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/client.hh>
#include <lixs/events.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

/* Must include errno before xs_wire.h, otherwise xsd_errors doesn't get defined */
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <utility>

extern "C" {
#include <xen/io/xs_wire.h>
}


namespace lixs {

template < typename CONNECTION >
class client : public CONNECTION, public functor {
public:
    template < typename... ARGS >
    client(xenstore& xs, event_mgr& emgr, ARGS&&... args);
    virtual ~client();

protected:
    char* cid;

    char* const abs_path;
    char* body;

private:
    class ev_cb_k : public lixs::ev_cb_k {
    public:
        ev_cb_k(client& client)
            : _client(client)
        { };

        void operator()(void);

        client& _client;
    };

    class watch_cb_k : public lixs::watch_cb_k {
    public:
        watch_cb_k(client& client, char* path, char* token, bool rel)
            : lixs::watch_cb_k(path, token), _client(client), rel(rel)
        { };

        void operator()(const std::string& path);

        client& _client;
        bool rel;
    };

    enum state {
        p_rx,
        rx_hdr,
        rx_body,
        p_tx,
        tx_hdr,
        tx_body,
        p_watch,
    };

    void operator()(void);

    void process(void);
    void handle_msg(void);
    void op_read(void);
    void op_write(void);
    void op_mkdir(void);
    void op_rm(void);
    void op_transaction_start(void);
    void op_transaction_end(void);
    void op_get_domain_path(void);
    void op_get_perms(void);
    void op_set_perms(void);
    void op_directory(void);
    void op_watch(void);
    void op_unwatch(void);
    void op_introduce_domain(void);
    void op_release_domain(void);
    void op_is_domain_introduced(void);

    char* get_path(void);

    void inline build_resp(const char* resp);
    void inline append_resp(const char* resp);
    void inline append_sep(void);
    void inline build_watch(const char* path, const char* token);
    void inline build_err(int err);
    void inline build_ack(void);

#ifdef DEBUG
    void inline print_msg(char* pre);
#endif

    xenstore& xs;
    event_mgr& emgr;
    ev_cb_k ev_cb;

    client::state state;
    std::map<std::string, watch_cb_k> watches;
    std::list<std::pair<std::string, watch_cb_k&> > fire_lst;

    /*
     * buff: [HEADER][/local/domain/<id>][BODY][/0]
     */
    char buff[sizeof(xsd_sockmsg) + 64 + XENSTORE_PAYLOAD_MAX + 1];
    struct xsd_sockmsg& msg;

    char* read_buff;
    char* write_buff;
    int read_bytes;
    int write_bytes;
};

template < typename CONNECTION >
void client<CONNECTION>::ev_cb_k::operator()(void)
{
    _client.process();
}

template < typename CONNECTION >
void client<CONNECTION>::watch_cb_k::operator()(const std::string& path)
{
    if (_client.state == rx_hdr) {
        _client.build_watch(path.c_str() + (rel ? _client.body - _client.abs_path : 0), token.c_str());
#ifdef DEBUG
        _client.print_msg((char*)">");
#endif

        _client.write_buff = reinterpret_cast<char*>(&_client.msg);
        _client.write_bytes = sizeof(_client.msg);
        if (!_client.write(_client.write_buff, _client.write_bytes)) {
            _client.state = tx_hdr;
            return;
        }

        _client.write_buff = _client.body;
        _client.write_bytes = _client.msg.len;
        if (!_client.write(_client.write_buff, _client.write_bytes)) {
            _client.state = tx_body;
        }
    } else {
        _client.fire_lst.push_back(
                std::pair<std::string, watch_cb_k&>(path, *this));
    }
}

template < typename CONNECTION >
template < typename... ARGS >
client<CONNECTION>::client(xenstore& xs, event_mgr& emgr, ARGS&&... args)
    : CONNECTION(*this, emgr, std::forward<ARGS>(args)...),
    cid((char*)"X"), abs_path(buff + sizeof(xsd_sockmsg)), body(abs_path),
    xs(xs), emgr(emgr), ev_cb(*this),
    state(p_rx), msg(*((xsd_sockmsg*)buff))
{
    emgr.enqueue_event(ev_cb);
}

template < typename CONNECTION >
client<CONNECTION>::~client()
{
    typename std::map<std::string, watch_cb_k>::iterator it;

    for (it = watches.begin(); it != watches.end(); it++) {
        xs.unwatch(it->second);
    }
}

template < typename CONNECTION >
void client<CONNECTION>::operator()(void)
{
    process();
    if (!CONNECTION::is_alive()) {
        delete this;
    }
}

template < typename CONNECTION >
void client<CONNECTION>::process(void)
{
    bool ret;
    bool yield = false;

    while (!yield && CONNECTION::is_alive()) {
        switch(state) {
            case p_rx:
                read_buff = reinterpret_cast<char*>(&msg);
                read_bytes = sizeof(msg);

                state = rx_hdr;
                break;

            case rx_hdr:
                ret = CONNECTION::read(read_buff, read_bytes);
                if (ret == false) {
                    yield = true;
                    break;
                }

                read_buff = body;
                read_bytes = msg.len;

                state = rx_body;
                break;

            case rx_body:
                ret = CONNECTION::read(read_buff, read_bytes);

                if (ret == false) {
                    yield = true;
                    break;
                }

#ifdef DEBUG
                print_msg((char*)"<");
#endif
                handle_msg();
#ifdef DEBUG
                print_msg((char*)">");
#endif

                state = p_tx;
                break;

            case p_tx:
                write_buff = reinterpret_cast<char*>(&msg);
                write_bytes = sizeof(msg);

                state = tx_hdr;
                break;

            case tx_hdr:
                ret = CONNECTION::write(write_buff, write_bytes);

                if (ret == false) {
                    yield = true;
                    break;
                }

                write_buff = body;
                write_bytes = msg.len;

                state = tx_body;
                break;

            case tx_body:
                ret = CONNECTION::write(write_buff, write_bytes);

                if (ret == false) {
                    yield = true;
                    break;
                }

                state = p_watch;
                break;

            case p_watch:
                if (fire_lst.empty()) {
                    state = p_rx;
                } else {
                    std::pair<std::string, watch_cb_k&>& e = fire_lst.front();

                    build_watch(e.first.c_str() + (e.second.rel ? body - abs_path : 0), e.second.token.c_str());
#ifdef DEBUG
                    print_msg((char*)">");
#endif

                    fire_lst.pop_front();
                    state = p_tx;
                }
                break;
        }
    }
}

template < typename CONNECTION >
void client<CONNECTION>::handle_msg(void)
{
    /* FIXME: This is a quick fix, need to analyse this better */
    /* Ensure the body is null terminated */
    body[msg.len] = '\0';

    switch (msg.type) {
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
            printf("XS_DEBUG\n");
            build_ack();
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
            op_introduce_domain();
        break;

        case XS_IS_DOMAIN_INTRODUCED:
            op_is_domain_introduced();
        break;

        case XS_RELEASE:
            op_release_domain();
        break;

        case XS_GET_DOMAIN_PATH:
            op_get_domain_path();
        break;

        case XS_RESUME:
            printf("client: XS_RESUME\n");
            build_err(ENOSYS);
        break;

        case XS_SET_TARGET:
            printf("client: XS_SET_TARGET\n");
            build_err(ENOSYS);
        break;

        case XS_RESET_WATCHES:
            printf("client: XS_RESET_WATCHES\n");
            build_err(ENOSYS);
        break;

        default:
            build_err(EINVAL);
        break;
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_read(void)
{
    int ret;
    std::string res;

    ret = xs.read(msg.tx_id, get_path(), res);

    if (ret == 0) {
        build_resp(res.c_str());
    } else {
        build_err(ret);
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_write(void)
{
    int ret;

    ret = xs.write(msg.tx_id, get_path(), body + strlen(body) + 1);

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_mkdir(void)
{
    int ret;

    ret = xs.mkdir(msg.tx_id, get_path());

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_rm(void)
{
    int ret;

    ret = xs.rm(msg.tx_id, get_path());

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_transaction_start(void)
{
    unsigned int tid;
    char id_str[32];

    xs.transaction_start(&tid);

    snprintf(id_str, 32, "%u", tid);
    build_resp(id_str);
}

template < typename CONNECTION >
void client<CONNECTION>::op_transaction_end(void)
{
    int ret;

    ret = xs.transaction_end(msg.tx_id, strcmp(body, "T") == 0);

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_get_domain_path(void)
{
    char buff[32];

    xs.get_domain_path(body, buff);

    build_resp(buff);
}

template < typename CONNECTION >
void client<CONNECTION>::op_get_perms(void)
{
    build_resp("b0");
}

template < typename CONNECTION >
void client<CONNECTION>::op_set_perms(void)
{
    build_ack();
}

template < typename CONNECTION >
void client<CONNECTION>::op_directory(void)
{
    std::set<std::string> resp;
    std::set<std::string>::iterator it;

    xs.directory(msg.tx_id, get_path(), resp);

    build_resp("");

    for (it = resp.begin(); it != resp.end(); it++) {
        append_resp((*it).c_str());
        append_sep();
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_watch(void)
{
    typename std::map<std::string, watch_cb_k>::iterator it;

    char* path = get_path();

    it = watches.find(path);
    if (it == watches.end()) {
        it = watches.insert(
                std::pair<std::string, watch_cb_k>(
                    path, watch_cb_k(*this, path, body + strlen(body) + 1, path != body))).first;
        xs.watch(it->second);
    }

    build_ack();
}

template < typename CONNECTION >
void client<CONNECTION>::op_unwatch(void)
{
    typename std::map<std::string, watch_cb_k>::iterator it;
    it = watches.find(get_path());

    if (it != watches.end()) {
        xs.unwatch(it->second);
        watches.erase(it);
        build_ack();
    } else {
        build_err(ENOENT);
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_introduce_domain(void)
{
    char* arg2 = body + strlen(body) + 1;
    char* arg3 = arg2 + strlen(arg2) + 1;

    xs.introduce_domain(atoi(body), atoi(arg2), atoi(arg3));

    build_ack();
}

template < typename CONNECTION >
void client<CONNECTION>::op_is_domain_introduced(void)
{
    bool exists;

    xs.exists_domain(atoi(body), exists);

    if (exists) {
        build_resp("T");
        append_sep();
    } else {
        build_resp("F");
        append_sep();
    }
}

template < typename CONNECTION >
void client<CONNECTION>::op_release_domain(void)
{
    xs.release_domain(atoi(body));

    build_ack();
}

template < typename CONNECTION >
char* client<CONNECTION>::get_path()
{
    if (body[0] == '/' || body[0] == '@') {
        return body;
    } else {
        return abs_path;
    }
}

template < typename CONNECTION >
void inline client<CONNECTION>::build_resp(const char* resp)
{
    /* FIXME: buffer will overflow if resp to big */

    msg.len = strlen(resp);
    memcpy(body, resp, msg.len);
}

template < typename CONNECTION >
void inline client<CONNECTION>::append_resp(const char* resp)
{
    int len = strlen(resp);

    memcpy(body + msg.len, resp, len);
    msg.len += len;
}

template < typename CONNECTION >
void inline client<CONNECTION>::append_sep(void)
{
    body[msg.len++] = '\0';
}

template < typename CONNECTION >
void inline client<CONNECTION>::build_watch(const char* path, const char* token)
{
    int path_len = strlen(path);
    int token_len = strlen(token);

    msg.type = XS_WATCH_EVENT;
    msg.req_id = 0;
    msg.tx_id = 0;

    memcpy(body, path, path_len);
    body[path_len] = '\0';
    memcpy(body + path_len + 1, token, token_len);
    body[path_len + 1 + token_len] = '\0';

    msg.len = path_len + token_len + 2;
}

template < typename CONNECTION >
void inline client<CONNECTION>::build_err(int err)
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

    msg.len = strlen(resp) + 1;
    msg.type = XS_ERROR;
    memcpy(body, resp, msg.len);
}

template < typename CONNECTION >
void inline client<CONNECTION>::build_ack(void)
{
    msg.len = 3;
    memcpy(body, "OK", 3);
}

#ifdef DEBUG
template < typename CONNECTION >
void inline client<CONNECTION>::print_msg(char* pre)
{
    unsigned int i;
    char c;

    body[msg.len] = '\0';

    printf("%4s %s { type = %2d, req_id = %d, tx_id = %d, len = %d, msg = ",
            cid, pre, msg.type, msg.req_id, msg.tx_id, msg.len);

    c = '"';
    for (i = 0; i < msg.len; i += strlen(body + i) + 1) {
        printf("%c%s", c, body + i);
        c = ' ';
    }

    printf("%s%s\" }\n", i == 0 ? "\"" : "", i > 0 && i == msg.len ? " " : "");
}
#endif

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

