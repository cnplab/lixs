#ifndef __LIXS_UNIX_SERVER_HH__
#define __LIXS_UNIX_SERVER_HH__

#include <lixs/iomux.hh>
#include <lixs/server.hh>
#include <lixs/xenstore.hh>

#include <string>


namespace lixs {

class unix_server : public server {
public:
    unix_server(iomux& io, xenstore& xs, std::string rw_path, std::string ro_path);
    ~unix_server();


private:
    class sviok : public fd_cb {
    public:
        sviok (unix_server& _server)
            : server(_server), fd(-1)
        { };

        void handle(const fd_cb::fd_ev& events) { server.handle(fd); };

        unix_server& server;
        int fd;
    };

private:
    void handle(int fd);

private:
    iomux& io;
    xenstore& xs;

    std::string rw_path;
    sviok rw_iok;

    std::string ro_path;
    sviok ro_iok;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_SERVER_HH__ */

