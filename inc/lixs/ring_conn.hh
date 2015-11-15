#ifndef __LIXS_RING_CONN_HH__
#define __LIXS_RING_CONN_HH__

#include <lixs/iomux.hh>

#include <cerrno>
#include <utility>

extern "C" {
#include <xenctrl.h>
#include <xen/io/xs_wire.h>
}


namespace lixs {

class ring_conn_base : public io_cb {
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

private:
    void operator()(bool read, bool write);

    bool read_chunck(char*& buff, int& bytes);
    bool write_chunck(char*& buff, int& bytes);

private:
    iomux& io;

    domid_t domid;
    evtchn_port_t local_port;
    evtchn_port_t remote_port;

    xc_evtchn *xce_handle;
    xenstore_domain_interface* interface;
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

