#ifndef __LIXS_RING_CONN_HH__
#define __LIXS_RING_CONN_HH__

#include <lixs/iomux.hh>

#include <cerrno>
#include <memory>
#include <stdexcept>
#include <utility>

extern "C" {
#include <xenctrl.h>
#include <xen/io/xs_wire.h>
}


namespace lixs {

class ring_conn_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ring_conn_cb;

class ring_conn_base {
private:
    friend ring_conn_cb;

protected:
    ring_conn_base(iomux& io, domid_t domid,
            evtchn_port_t port, xenstore_domain_interface* interface);
    virtual ~ring_conn_base();

protected:
    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);

    void need_rx(void);
    void need_tx(void);

    virtual void process_rx(void) = 0;
    virtual void process_tx(void) = 0;
    virtual void conn_dead(void) = 0;

private:
    bool read_chunk(char*& buff, int& bytes);
    bool write_chunk(char*& buff, int& bytes);

private:
    iomux& io;

    int fd;
    bool ev_read;
    bool ev_write;

    std::shared_ptr<ring_conn_cb> cb;

    domid_t domid;
    evtchn_port_t local_port;
    evtchn_port_t remote_port;

    xc_evtchn *xce_handle;
    xenstore_domain_interface* interface;
};

class ring_conn_cb {
public:
    ring_conn_cb(ring_conn_base& conn);

public:
    static void callback(bool read, bool write, std::weak_ptr<ring_conn_cb> ptr);

private:
    ring_conn_base& conn;
};


template < typename MAPPER >
class ring_conn : public MAPPER, public ring_conn_base {
protected:
    template < typename... ARGS >
    ring_conn(iomux& io, domid_t domid, evtchn_port_t port, ARGS&&... args);
    virtual ~ring_conn();
};


template < typename MAPPER >
template < typename... ARGS >
ring_conn<MAPPER>::ring_conn(iomux& io, domid_t domid, evtchn_port_t port, ARGS&&... args)
    : MAPPER(domid, std::forward<ARGS>(args)...),
    ring_conn_base(io, domid, port, MAPPER::interface)
{
}

template < typename MAPPER >
ring_conn<MAPPER>::~ring_conn()
{
}

} /* namespace lixs */

#endif /* __LIXS_RING_CONN_HH__ */

