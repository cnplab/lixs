#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/iomux.hh>
#include <lixs/xenstore.hh>

#include <stdint.h>
/* Must include errno before xs_wire.h, otherwise xsd_errors doesn't get defined */
#include <errno.h>

extern "C" {
#include <xen/io/xs_wire.h>
}


namespace lixs {

class client : public ev_cb, public fd_cb {
public:
    void run(void);
    void handle(const fd_cb::fd_ev& events);

protected:
    client(iomux& io, xenstore& xs);
    virtual ~client();

    virtual void process_events(const fd_cb::fd_ev& events);
    virtual bool read(char*& buff, int& bytes) = 0;
    virtual bool write(char*& buff, int& bytes) = 0;

    iomux& io;

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

    xenstore& xs;

    char buff[sizeof(xsd_sockmsg) + XENSTORE_PAYLOAD_MAX];
    struct xsd_sockmsg& msg;
    char* const body;
};

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

