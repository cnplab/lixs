#ifndef __LIXS_UNIX_SERVER_HH__
#define __LIXS_UNIX_SERVER_HH__

#include <lixs/iomux.hh>
#include <lixs/server.hh>

#include <string>


namespace lixs {

class unix_server : public server {
public:
    unix_server(iomux& io, std::string path, std::string ro_path);
    ~unix_server();


private:
    static void handle_server(iomux::ptr* ptr);
    static void handle_server_ro(iomux::ptr* ptr);

    void handle(int fd);


private:
    int sock_fd;
    std::string sock_path;
    iomux::ptr iomux_ptr;

    int sock_ro_fd;
    std::string sock_ro_path;
    iomux::ptr iomux_ro_ptr;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_SERVER_HH__ */

