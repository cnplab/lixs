#include <lixs/client.hh>
#include <lixs/store.hh>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <errno.h>

extern "C" {
#include <xen/io/xs_wire.h>
}


unsigned int lixs::client::trans_id = 1;

lixs::client::client(iomux& io, store& st)
    : io(io), alive(true), state(p_init), st(st),
    msg(*((xsd_sockmsg*)buff)), body(buff + sizeof(xsd_sockmsg))
{
    io.once(*this);
}

lixs::client::~client()
{
}


void lixs::client::init_store(store& st)
{
    st.ensure("/");
}


void lixs::client::run(void)
{
    process();
}

void lixs::client::handle(const ioev& events)
{
    process_events(events);
    process();

    if (!alive) {
        delete this;
    }
}

void lixs::client::process_events(const ioev& events)
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
    const char* res;

    if (msg.tx_id) {
        res = st.read(msg.tx_id, body);
    } else {
        res = st.read(body);
    }

    if (res) {
        build_resp(res);
    } else {
        build_err(ENOENT);
    }
}

void lixs::client::op_write(void)
{
    unsigned int i;
    unsigned int len = strlen(body);

    i = 1;
    do {
        i += strcspn(body + i, "/");
        if (i < len) {
            body[i] = '\0';
            if (msg.tx_id) {
                st.ensure(msg.tx_id, body);
            } else {
                st.ensure(body);
            }
            body[i] = '/';
        }

        i++;
    } while(i < len);

    if (msg.tx_id) {
        st.write(msg.tx_id, body, body + strlen(body) + 1);
    } else {
        st.write(body, body + strlen(body) + 1);
    }

    build_ack();
}

void lixs::client::op_mkdir(void)
{
    if (msg.tx_id) {
        if (!st.read(msg.tx_id, body)) {
            st.write(msg.tx_id, body, "");
        }
    } else {
        if (!st.read(body)) {
            st.write(body, "");
        }
    }

    build_ack();
}

void lixs::client::op_rm(void)
{
    if (msg.tx_id) {
        st.del(body);
    } else {
        st.del(msg.tx_id, body);
    }

    build_ack();
}

void lixs::client::op_transaction_start(void)
{
    unsigned int id;
    char id_str[32];

    id = trans_id++;

    st.branch(id);
    snprintf(id_str, 32, "%u", id);
    build_resp(id_str);
}

void lixs::client::op_transaction_end(void)
{
    if (strcmp(body, "T") == 0) {
        if (st.merge(msg.tx_id)) {
            build_ack();
        } else {
            build_err(EAGAIN);
        }
    } else {
        st.abort(msg.tx_id);
        build_ack();
    }
}

void lixs::client::op_get_domain_path(void)
{
    char buff[32];

    sprintf(buff, "/local/domain/%s", body);

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
    int nresp;
    const char* resp[1024];

    nresp = st.get_childs(body, resp, 1024);

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

