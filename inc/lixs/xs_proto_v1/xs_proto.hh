#ifndef __LIXS_XS_PROTO_V1_XS_PROTO_HH__
#define __LIXS_XS_PROTO_V1_XS_PROTO_HH__

#include <lixs/domain_mgr.hh>
#include <lixs/permissions.hh>
#include <lixs/watch.hh>
#include <lixs/xenstore.hh>

#include <cstring>
#include <list>
#include <string>
#include <utility>

extern "C" {
#include <xen/xen.h>
#include <xen/io/xs_wire.h>
}


namespace lixs {
namespace xs_proto_v1 {

/* /local/domain/<id> */
const int dom_path_length_max = 35;
std::string get_dom_path(domid_t domid);


class xs_proto_base;


class wire {
public:
    wire(std::string dom_path);

public:
    void sanitize_input(void);
    operator std::string () const;

public:
    struct xsd_sockmsg hdr;

    /* make this pointers const */
    char* const abs_path;
    char* const body;

private:
    /*
     * buff: [dom_path][BODY][/0]
     *
     * * dom_path is used to handle relative paths
     * * /0 is just space to simplify the algorithms the write to the buffer
     */
    char buff[dom_path_length_max + XENSTORE_PAYLOAD_MAX + 1];
};


class message {
public:
    message(uint32_t type, uint32_t req_id, uint32_t tx_id,
            std::list<std::string> body, bool terminator);

public:
    const uint32_t type;
    const uint32_t req_id;
    const uint32_t tx_id;
    const std::list<std::string> body;
    const bool terminator;
};


class watch_cb : public lixs::watch_cb {
public:
    watch_cb(xs_proto_base& proto,
            const std::string& path, const std::string& token, bool relative);

public:
    void operator()(const std::string& fire_path);

private:
    xs_proto_base& proto;
    const bool relative;
};

typedef std::pair<std::string, std::string> watch_key;
typedef std::map<watch_key, watch_cb> watch_map;


enum class io_state {
    p,
    hdr,
    body,
};


class xs_proto_base {
protected:
    friend watch_cb;

protected:
    xs_proto_base(domid_t domid, xenstore& xs, domain_mgr& dmgr);
    virtual ~xs_proto_base();

protected:
    virtual void process_rx(void) = 0;
    virtual void process_tx(void) = 0;
    virtual std::string cid(void) = 0;

protected:
    void handle_rx(void);
    bool prepare_tx(void);

private:
    void op_read(void);
    void op_write(void);
    void op_mkdir(void);
    void op_rm(void);
    void op_directory(void);
    void op_transaction_start(void);
    void op_transaction_end(void);
    void op_get_domain_path(void);
    void op_get_perms(void);
    void op_set_perms(void);
    void op_watch(void);
    void op_unwatch(void);
    void op_introduce(void);
    void op_release(void);
    void op_is_domain_introduced(void);
    void op_unimplemented(void);

    void perm2str(const permission& perm, std::string& str);
    bool str2perm(const std::string& str, permission& perm);
    std::string err2str(int err);
    std::string path2rel(std::string path);

    char* get_arg1(void);
    char* get_arg2(void);
    char* get_arg3(void);
    char* get_next_arg(char* curr);
    char* get_path(void);

    template<typename int_t>
    int get_int(const char* arg, int_t& number);

    void build_hdr(uint32_t type, uint32_t req_id, uint32_t tx_id);
    bool build_body(std::string elem, bool terminator);
    bool build_body(std::list<std::string> elems, bool terminator);

protected:
    domid_t domid;
    std::string dom_path;

    wire rx_msg;
    wire tx_msg;

    std::list<message> tx_queue;
    watch_map watches;

    xenstore& xs;
    domain_mgr& dmgr;
};


template < typename CONNECTION >
class xs_proto: public CONNECTION, public xs_proto_base {
protected:
    template < typename... ARGS >
    xs_proto(domid_t domid, xenstore& xs, domain_mgr& dmgr, ARGS&&... args);
    ~xs_proto();

protected:
    virtual std::string cid(void) = 0;

private:
    void process_rx(void);
    void process_tx(void);

private:
    io_state rx_state;
    io_state tx_state;

    char* rx_buff;
    char* tx_buff;
    int rx_bytes;
    int tx_bytes;
};

template < typename CONNECTION >
template < typename... ARGS >
xs_proto<CONNECTION>::xs_proto(domid_t domid, xenstore& xs, domain_mgr& dmgr, ARGS&&... args)
    : CONNECTION(std::forward<ARGS>(args)...), xs_proto_base(domid, xs, dmgr),
    rx_state(io_state::p), tx_state(io_state::p)
{
    CONNECTION::need_rx();
}

template < typename CONNECTION >
xs_proto<CONNECTION>::~xs_proto()
{
    for (auto& w : watches) {
        xs.watch_del(w.second);
    }
}

template < typename CONNECTION >
void xs_proto<CONNECTION>::process_rx(void)
{
    while (true) {
        switch(rx_state) {
            case io_state::p:
                rx_buff = reinterpret_cast<char*>(&(rx_msg.hdr));
                rx_bytes = sizeof(rx_msg.hdr);

                rx_state = io_state::hdr;
                break;

            case io_state::hdr:
                if (CONNECTION::read(rx_buff, rx_bytes) == false) {
                    return;
                }

                rx_buff = rx_msg.body;
                rx_bytes = rx_msg.hdr.len;

                rx_state = io_state::body;
                break;

            case io_state::body:
                if (CONNECTION::read(rx_buff, rx_bytes) == false) {
                    return;
                }

                rx_msg.sanitize_input();

#if DEBUG
                printf("LiXS: [%4s] %s %s\n", cid().c_str(), "<",
                        static_cast<std::string>(rx_msg).c_str());
#endif

                handle_rx();

                rx_state = io_state::p;
                break;
        }
    }
}

template < typename CONNECTION >
void xs_proto<CONNECTION>::process_tx(void)
{
    while (true) {
        switch(tx_state) {
            case io_state::p:
                if (prepare_tx() == false) {
                    return;
                }

#if DEBUG
                printf("LiXS: [%4s] %s %s\n", cid().c_str(), ">",
                        static_cast<std::string>(tx_msg).c_str());
#endif

                tx_buff = reinterpret_cast<char*>((&tx_msg.hdr));
                tx_bytes = sizeof(tx_msg.hdr);

                tx_state = io_state::hdr;
                break;

            case io_state::hdr:
                if (CONNECTION::write(tx_buff, tx_bytes) == false) {
                    return;
                }

                tx_buff = tx_msg.body;
                tx_bytes = tx_msg.hdr.len;

                tx_state = io_state::body;
                break;

            case io_state::body:
                if (CONNECTION::write(tx_buff, tx_bytes) == false) {
                    return;
                }

                tx_state = io_state::p;
                break;
        }
    }
}

} /* namespace xs_proto_v1 */
} /* namespace lixs */

#endif /* __LIXS_XS_PROTO_V1_XS_PROTO_HH__ */

