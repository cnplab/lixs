#include <lixs/client.hh>
#include <lixs/store.hh>

#include <cstddef>
#include <cstdio>
#include <cstring>


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

    if (!alive)
        delete this;
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

                printf("{ type = %d, req_id = %d, tx_id = %d, len = %d }\n",
                        msg.type, msg.req_id, msg.tx_id, msg.len);

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

                fprintf(stdout, "msg = %s ", body);
                //fprintf(stdout, "%s\n", buff + strlen(buff) + 1);
                fprintf(stdout, "\n");

                msg.len = 4;

                memcpy(body, "lixs", 5);

                write_buff = buff;
                write_bytes = sizeof(msg) + 4;

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
        break;

        case XS_READ:
        break;

        case XS_WRITE:
        break;

        case XS_MKDIR:
        break;

        case XS_RM:
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

