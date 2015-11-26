#ifndef __LIXS_SOCK_CONN_HH__
#define __LIXS_SOCK_CONN_HH__

#include <lixs/iomux.hh>

#include <memory>
#include <stdexcept>


namespace lixs {

class sock_conn_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class sock_conn_cb;

class sock_conn {
private:
    friend sock_conn_cb;

protected:
    sock_conn(iomux& io, int fd);
    virtual ~sock_conn();

protected:
    bool read(char*& buff, int& bytes);
    bool write(char*& buff, int& bytes);

    void need_rx(void);
    void need_tx(void);

    virtual void process_rx(void) = 0;
    virtual void process_tx(void) = 0;
    virtual void conn_dead(void) = 0;

private:
    iomux& io;

    int fd;
    bool ev_read;
    bool ev_write;

    bool alive;

    std::shared_ptr<sock_conn_cb> cb;
};

class sock_conn_cb {
public:
    sock_conn_cb(sock_conn& conn);

public:
    static void callback(bool read, bool write, std::weak_ptr<sock_conn_cb> ptr);

private:
    sock_conn& conn;
};

} /* namespace lixs */

#endif /* __LIXS_SOCK_CONN_HH__ */

