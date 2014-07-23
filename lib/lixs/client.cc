#include <lixs/client.hh>

#include <cstddef>
#include <cstdio>
#include <cstring>


lixs::client::client(iomux& io)
    : io(io), state(p_init)
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
}

void lixs::client::process_events(const ioev& events)
{
    /* Provide empty implementation */
}

void lixs::client::process(void)
{
    bool ret;
    bool done = false;

    while (!done) {
        switch(state) {
            case p_init:
                read_buff = reinterpret_cast<char*>(&msg);
                read_bytes = sizeof(msg);

                state = rx_hdr;
                break;

            case rx_hdr:
                ret = read(read_buff, read_bytes);
                if (ret == false) {
                    done = true;
                    break;
                }

                printf("{ type = %d, req_id = %d, tx_id = %d, len = %d }\n",
                        msg.type, msg.req_id, msg.tx_id, msg.len);

                buff = new char[msg.len];

                read_buff = buff;
                read_bytes = msg.len;

                state = rx_body;
                break;

            case rx_body:
                ret = read(read_buff, read_bytes);

                if (ret == false) {
                    done = true;
                    break;
                }

                fprintf(stdout, "msg = %s ", buff);
                //fprintf(stdout, "%s\n", buff + strlen(buff) + 1);
                fprintf(stdout, "\n");

                msg.len = 4;

                write_buff = buff;
                write_bytes = 0;

                memcpy(write_buff, &msg, sizeof(msg));
                write_bytes += sizeof(msg);
                write_buff += sizeof(msg);

                memcpy(write_buff, "lixs", 5);
                write_bytes += 5;
                write_buff = buff;

                state = tx_resp;
                break;

            case tx_resp:
                ret = write(write_buff, write_bytes);

                if (ret == false) {
                    done = true;
                    break;
                }

                delete[] buff;

                state = rx_hdr;
                break;
        }
    }
}

