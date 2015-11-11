#ifndef __LIXS_SOCK_CONN_HH__
#define __LIXS_SOCK_CONN_HH__

#include <lixs/iomux.hh>


namespace lixs {

class sock_conn : public fd_cb_k {
protected:
    sock_conn(iomux& io, int fd);
    virtual ~sock_conn();

protected:
    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);

    virtual void process_rx(void) = 0;
    virtual void process_tx(void) = 0;
    virtual void conn_dead(void) = 0;

private:
    void operator()(bool read, bool write);

private:
    iomux& io;

    bool alive;
};

} /* namespace lixs */

#endif /* __LIXS_SOCK_CONN_HH__ */

