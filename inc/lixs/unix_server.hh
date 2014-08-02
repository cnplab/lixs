#ifndef __LIXS_UNIX_SERVER_HH__
#define __LIXS_UNIX_SERVER_HH__

#include <lixs/events.hh>
#include <lixs/server.hh>
#include <lixs/xenstore.hh>

#include <string>


namespace lixs {

class xenstore;

class unix_server : public server {
public:
    unix_server(xenstore& xs, std::string rw_path, std::string ro_path);
    ~unix_server();


private:
    class fd_cb_k : public lixs::fd_cb_k {
    public:
        fd_cb_k (unix_server& server)
            : server(server)
        { };

        void operator()(bool read, bool write);

        unix_server& server;
    };

    xenstore& xs;

    std::string rw_path;
    fd_cb_k rw_cb;

    std::string ro_path;
    fd_cb_k ro_cb;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_SERVER_HH__ */

