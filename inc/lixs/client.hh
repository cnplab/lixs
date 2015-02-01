#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/events.hh>
#include <lixs/event_mgr.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <list>
#include <map>
#include <string>
#include <utility>

extern "C" {
#include <xen/io/xs_wire.h>
}


namespace lixs {

class client_base {
protected:
    struct msg {
        msg(char* buff)
            : hdr(*((xsd_sockmsg*)buff)), abs_path(buff + sizeof(xsd_sockmsg)), body(abs_path)
        { }

        struct xsd_sockmsg& hdr;
        char* abs_path;
        char* body;
    };

    class ev_cb_k : public lixs::ev_cb_k {
    public:
        ev_cb_k(client_base& client)
            : _client(client)
        { };

        void operator()(void);

        client_base& _client;
    };

    class watch_cb_k : public lixs::watch_cb_k {
    public:
        watch_cb_k(client_base& client, char* path, char* token, bool rel)
            : lixs::watch_cb_k(path, token), _client(client), rel(rel)
        { };

        void operator()(const std::string& path);

        client_base& _client;
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

    typedef std::map<std::string, watch_cb_k> watch_map;
    typedef std::list<std::pair<std::string, std::string> > fire_list;


    client_base(xenstore& xs, event_mgr& emgr);
    virtual ~client_base();

    virtual void operator()(void) = 0;
    virtual void process(void) = 0;
    virtual void watch_fired(const std::string& path, const std::string& token) = 0;

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
    void op_set_target(void);
    void op_restrict(void);
    void op_directory(void);
    void op_watch(void);
    void op_unwatch(void);
    void op_reset_watches(void);
    void op_introduce(void);
    void op_release(void);
    void op_is_domain_introduced(void);
    void op_debug(void);
    void op_resume(void);

    char* get_path(void);
    char* get_arg1(void);
    char* get_arg2(void);
    char* get_arg3(void);

    void build_resp(const char* resp);
    void append_resp(const char* resp);
    void append_sep(void);
    void build_watch(const char* path, const char* token);
    void build_err(int err);
    void build_ack(void);

#ifdef DEBUG
    void print_msg(char* pre);
#endif

    xenstore& xs;
    event_mgr& emgr;

    ev_cb_k ev_cb;
    client_base::state state;

    watch_map watches;
    fire_list to_fire;

    /*
     * buff: [HEADER][/local/domain/<id>][BODY][/0]
     */
    char buff[sizeof(xsd_sockmsg) + 35 + XENSTORE_PAYLOAD_MAX + 1];
    struct msg msg;

    char* cid;

    char* read_buff;
    char* write_buff;
    int read_bytes;
    int write_bytes;
};


template < typename CONNECTION >
class client : public client_base, public CONNECTION, public ev_cb_k {
public:
    template < typename... ARGS >
    client(xenstore& xs, event_mgr& emgr, ARGS&&... args);
    virtual ~client();

private:
    void operator()(void);
    void process(void);
    void watch_fired(const std::string& path, const std::string& token);
};

template < typename CONNECTION >
template < typename... ARGS >
client<CONNECTION>::client(xenstore& xs, event_mgr& emgr, ARGS&&... args)
    : client_base(xs, emgr), CONNECTION(*this, emgr, std::forward<ARGS>(args)...)
{
}

template < typename CONNECTION >
client<CONNECTION>::~client()
{
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
                read_buff = reinterpret_cast<char*>(&(msg.hdr));
                read_bytes = sizeof(msg.hdr);

                state = rx_hdr;
                break;

            case rx_hdr:
                ret = CONNECTION::read(read_buff, read_bytes);
                if (ret == false) {
                    yield = true;
                    break;
                }

                read_buff = msg.body;
                read_bytes = msg.hdr.len;

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
                write_buff = reinterpret_cast<char*>((&msg.hdr));
                write_bytes = sizeof(msg.hdr);

                state = tx_hdr;
                break;

            case tx_hdr:
                ret = CONNECTION::write(write_buff, write_bytes);

                if (ret == false) {
                    yield = true;
                    break;
                }

                write_buff = msg.body;
                write_bytes = msg.hdr.len;

                state = tx_body;
                break;

            case tx_body:
                ret = CONNECTION::write(write_buff, write_bytes);

                if (ret == false) {
                    yield = true;
                    break;
                }

                state = to_fire.empty() ? p_rx : p_watch;
                break;

            case p_watch:
                std::pair<std::string, std::string>& e = to_fire.front();
                build_watch(e.first.c_str(), e.second.c_str());
                to_fire.pop_front();

#ifdef DEBUG
                print_msg((char*)">");
#endif

                state = p_tx;
                break;
        }
    }
}

template < typename CONNECTION >
void client<CONNECTION>::watch_fired(const std::string& path, const std::string& token)
{
    if (state == rx_hdr) {
        build_watch(path.c_str(), token.c_str());
#ifdef DEBUG
        print_msg((char*)">");
#endif

        write_buff = reinterpret_cast<char*>(&msg.hdr);
        write_bytes = sizeof(msg.hdr);
        if (!CONNECTION::write(write_buff, write_bytes)) {
            state = tx_hdr;
            return;
        }

        write_buff = msg.body;
        write_bytes = msg.hdr.len;
        if (!CONNECTION::write(write_buff, write_bytes)) {
            state = tx_body;
        }
    } else {
        to_fire.push_back(std::pair<std::string, std::string>(path, token));
    }
}

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

