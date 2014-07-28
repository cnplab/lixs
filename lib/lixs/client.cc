#include <lixs/client.hh>
#include <lixs/store.hh>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <errno.h>

extern "C" {
#include <xen/io/xs_wire.h>
}


lixs::client::client(iomux& io, store& st)
    : io(io), alive(true), state(p_init), st(st),
    msg(*((xsd_sockmsg*)buff)), body(buff + sizeof(xsd_sockmsg))
{
    io.once(*this);
}

lixs::client::~client()
{
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

                handle_msg();

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
    body[msg.len] = '\0';

    if (strlen(body) < (msg.len - 1)) {
        printf("{ type = %2d, req_id = %d, tx_id = %d, len = %d, msg = \"%s %s\" }\n",
                msg.type, msg.req_id, msg.tx_id, msg.len, body, body + strlen(body) + 1);
    } else {
        printf("{ type = %2d, req_id = %d, tx_id = %d, len = %d, msg = \"%s\" }\n",
                msg.type, msg.req_id, msg.tx_id, msg.len, body);
    }

    switch (msg.type) {
        case XS_DIRECTORY:
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
        break;

        case XS_SET_PERMS:
        break;

        case XS_DEBUG:
        break;

        case XS_WATCH:
        break;

        case XS_UNWATCH:
        break;

        case XS_TRANSACTION_START:
        break;

        case XS_TRANSACTION_END:
        break;

        case XS_INTRODUCE:
        break;

        case XS_IS_DOMAIN_INTRODUCED:
        break;

        case XS_RELEASE:
        break;

        case XS_GET_DOMAIN_PATH:
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
    const char* res = st.read(body);

    if (res) {
        build_resp(res);
    } else {
        build_resp("");
    }
}

void lixs::client::op_write(void)
{
    st.write(body, body + strlen(body) + 1);

    build_ack();
}

void lixs::client::op_mkdir(void)
{
    if (!st.read(body)) {
        st.write(body, "");
    }

    build_ack();
}

void lixs::client::op_rm(void)
{
    st.del(body);

    build_ack();
}

void inline lixs::client::build_resp(const char* resp)
{
    msg.len = strlen(resp);
    memcpy(body, resp, msg.len);

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

