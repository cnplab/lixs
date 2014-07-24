#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/iomux.hh>

#include <stdint.h>

extern "C" {
#include <xen/io/xs_wire.h>
}


namespace lixs {

class client : public iok, public iokfd {
public:
    void run(void);
    void handle(const iokfd::ioev& events);


protected:
    client(iomux& io);
    virtual ~client();

    virtual void process_events(const iokfd::ioev& events);
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

    client::state state;

    char buff[sizeof(xsd_sockmsg) + XENSTORE_PAYLOAD_MAX];

    struct xsd_sockmsg& msg;
    char* const body;
};

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

