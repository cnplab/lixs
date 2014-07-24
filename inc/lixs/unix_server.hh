#ifndef __LIXS_UNIX_SERVER_HH__
#define __LIXS_UNIX_SERVER_HH__

#include <lixs/iomux.hh>
#include <lixs/server.hh>
#include <lixs/store.hh>

#include <string>


namespace lixs {

class unix_server : public server {
public:
    unix_server(iomux& io, store& st, std::string rw_path, std::string ro_path);
    ~unix_server();


private:
    class sviok : public iokfd {
    public:
        sviok (unix_server& _server)
            : server(_server), fd(-1)
        { };

        void handle(const iokfd::ioev& events) { server.handle(fd); };

        unix_server& server;
        int fd;
    };

private:
    void handle(int fd);

private:
    iomux& io;
    store& st;

    std::string rw_path;
    sviok rw_iok;

    std::string ro_path;
    sviok ro_iok;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_SERVER_HH__ */

