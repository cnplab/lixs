#include <lixs/client.hh>
#include <lixs/xenstore.hh>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <errno.h>

extern "C" {
#include <xen/io/xs_wire.h>
}


lixs::client::client(xenstore& xs)
    : xs(xs), fd_cb(*this), alive(true), state(p_init),
    msg(*((xsd_sockmsg*)buff)), body(buff + sizeof(xsd_sockmsg))
{
    xs.once(*this);
}

lixs::client::~client()
{
}


void lixs::client::run(void)
{
    process();
}

void lixs::client::fd_cb_k::operator()(bool read, bool write)
{
    _client.process_events(read, write);
    _client.process();

    if (!_client.alive) {
        delete &_client;
    }
}

void lixs::client::process_events(bool read, bool write)
{
    /* Provide empty implementation */
}

void lixs::client::process(void)
{
    bool ret;
    bool yield = false;

    while (!yield && alive) {
        switch(state) {
            case p_init:
                read_buff = reinterpret_cast<char*>(&msg);
                read_bytes = sizeof(msg);

                state = rx_hdr;
                break;

            case rx_hdr:
                ret = read(read_buff, read_bytes);
                if (ret == false) {
                    yield = true;
                    break;
                }

                read_buff = body;
                read_bytes = msg.len;

                state = rx_body;
                break;

            case rx_body:
                ret = read(read_buff, read_bytes);

                if (ret == false) {
                    yield = true;
                    break;
                }

                print_msg((char*)"<");
                handle_msg();
                print_msg((char*)">");

                state = tx_resp;
                break;

            case tx_resp:
                ret = write(write_buff, write_bytes);

                if (ret == false) {
                    yield = true;
                    break;
                }

                state = p_init;
                break;
        }
    }
}

void lixs::client::handle_msg(void)
{
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
        break;

        case XS_WATCH:
        break;

        case XS_UNWATCH:
        break;

        case XS_TRANSACTION_START:
            op_transaction_start();
        break;

        case XS_TRANSACTION_END:
            op_transaction_end();
        break;

        case XS_INTRODUCE:
        break;

        case XS_IS_DOMAIN_INTRODUCED:
        break;

        case XS_RELEASE:
        break;

        case XS_GET_DOMAIN_PATH:
            op_get_domain_path();
        break;

        case XS_RESUME:
        break;

        case XS_SET_TARGET:
        break;

        case XS_RESET_WATCHES:
        break;

        default:
        break;
    }
}

void lixs::client::op_read(void)
{
    int ret;
    const char* res;

    ret = xs.read(msg.tx_id, body, &res);

    if (ret == 0) {
        build_resp(res);
    } else {
        build_err(ret);
    }
}

void lixs::client::op_write(void)
{
    int ret;

    ret = xs.write(msg.tx_id, body, body + strlen(body) + 1);

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client::op_mkdir(void)
{
    int ret;

    ret = xs.mkdir(msg.tx_id, body);

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client::op_rm(void)
{
    int ret;

    ret = xs.rm(msg.tx_id, body);

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client::op_transaction_start(void)
{
    unsigned int tid;
    char id_str[32];

    xs.transaction_start(&tid);

    snprintf(id_str, 32, "%u", tid);
    build_resp(id_str);
}

void lixs::client::op_transaction_end(void)
{
    int ret;

    ret = xs.transaction_end(msg.tx_id, strcmp(body, "T") == 0);

    if (ret == 0) {
        build_ack();
    } else {
        build_err(ret);
    }
}

void lixs::client::op_get_domain_path(void)
{
    char buff[32];

    xs.get_domain_path(body, buff);

    build_resp(buff);
}

void lixs::client::op_get_perms(void)
{
    build_resp("b0");
}

void lixs::client::op_set_perms(void)
{
    build_ack();
}

void lixs::client::op_directory(void)
{
    int nresp = 1024;
    const char* resp[1024];

    xs.directory(msg.tx_id, body, resp, &nresp);

    build_resp("");

    for (int i = 0; i < nresp; i++) {
        append_resp(resp[i]);
        append_sep();
    }
}

void inline lixs::client::build_resp(const char* resp)
{
    msg.len = strlen(resp);
    memcpy(body, resp, msg.len);

    write_buff = buff;
    write_bytes = sizeof(msg) + msg.len;
}

void inline lixs::client::append_resp(const char* resp)
{
    int len = strlen(resp);

    memcpy(body + msg.len, resp, len);
    msg.len += len;

    write_buff = buff;
    write_bytes = sizeof(msg) + msg.len;
}

void inline lixs::client::append_sep(void)
{
    body[msg.len++] = '\0';

    write_buff = buff;
    write_bytes = sizeof(msg) + msg.len;
}

void inline lixs::client::build_err(int err)
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

    msg.len = strlen(resp);
    msg.type = XS_ERROR;
    memcpy(body, resp, msg.len);

    write_buff = buff;
    write_bytes = sizeof(msg) + msg.len;
}

void inline lixs::client::build_ack(void)
{
    msg.len = 2;
    memcpy(body, "OK", 2);

    write_buff = buff;
    write_bytes = sizeof(msg) + 2;
}

void inline lixs::client::print_msg(char* pre)
{
    unsigned int i;
    char c;

    body[msg.len] = '\0';

    printf("%s { type = %2d, req_id = %d, tx_id = %d, len = %d, msg = ",
            pre, msg.type, msg.req_id, msg.tx_id, msg.len);

    c = '"';
    for (i = 0; i < msg.len; i += strlen(body + i) + 1) {
        printf("%c%s", c, body + i);
        c = ' ';
    }

    printf("%s%s\" }\n", i == 0 ? "\"" : "", i > 0 && i == msg.len ? " " : "");
}

