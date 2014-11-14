#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/events.hh>
#include <lixs/event_mgr.hh>

/* Must include errno before xs_wire.h, otherwise xsd_errors doesn't get defined */
#include <cerrno>
#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <utility>

extern "C" {
#include <xen/io/xs_wire.h>
}


namespace lixs {

class xenstore;

class client {
protected:
    class fd_cb_k : public lixs::fd_cb_k {
    public:
        fd_cb_k (client& client)
            : _client(client)
        { };

        void operator()(bool read, bool write);

        client& _client;
    };

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

    client(xenstore& xs, event_mgr& emgr);
    virtual ~client();

    virtual void process_events(bool read, bool write);
    virtual bool read(char*& buff, int& bytes) = 0;
    virtual bool write(char*& buff, int& bytes) = 0;

    char* cid;

    xenstore& xs;
    event_mgr& emgr;
    fd_cb_k fd_cb;
    ev_cb_k ev_cb;

    bool alive;

    char* read_buff;
    char* write_buff;
    int read_bytes;
    int write_bytes;

    char* const abs_path;
    char* body;

private:
    enum state {
        p_rx,
        rx_hdr,
        rx_body,
        p_tx,
        tx_hdr,
        tx_body,
        p_watch,
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
    void op_watch(void);
    void op_unwatch(void);
    void op_introduce_domain(void);
    void op_release_domain(void);

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

    client::state state;
    std::map<std::string, watch_cb_k> watches;
    std::list<std::pair<std::string, watch_cb_k&> > fire_lst;

    /*
     * buff: [HEADER][/local/domain/<id>][BODY][/0]
     */
    char buff[sizeof(xsd_sockmsg) + 64 + XENSTORE_PAYLOAD_MAX + 1];
    struct xsd_sockmsg& msg;
};

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

