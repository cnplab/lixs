#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/xenstore.hh>

#include <stdint.h>
/* Must include errno before xs_wire.h, otherwise xsd_errors doesn't get defined */
#include <errno.h>

extern "C" {
#include <xen/io/xs_wire.h>
}


namespace lixs {

class client : public ev_cb {
protected:
    class fd_cb_k : public lixs::fd_cb_k {
    public:
        fd_cb_k (client& client)
            : _client(client)
        { };

        void operator()(bool read, bool write);

        client& _client;
    };

    client(xenstore& xs);
    virtual ~client();

    virtual void process_events(bool read, bool write);
    virtual bool read(char*& buff, int& bytes) = 0;
    virtual bool write(char*& buff, int& bytes) = 0;

    void run(void);

    xenstore& xs;
    fd_cb_k fd_cb;

    bool alive;

    char* read_buff;
    char* write_buff;
    int read_bytes;
    int write_bytes;


private:
    enum state {
        p_init,
        rx_hdr,
        rx_body,
        tx_resp,
    };

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

    void inline build_resp(const char* resp);
    void inline append_resp(const char* resp);
    void inline append_sep(void);
    void inline build_err(int err);
    void inline build_ack(void);

    void inline print_msg(char* pre);

    client::state state;

    char buff[sizeof(xsd_sockmsg) + XENSTORE_PAYLOAD_MAX];
    struct xsd_sockmsg& msg;
    char* const body;
};

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

