#ifndef __LIXS_UNIX_CLIENT_HH__
#define __LIXS_UNIX_CLIENT_HH__

#include <lixs/client.hh>
#include <lixs/iomux.hh>


namespace lixs {

class unix_client : public client {
public:
    static void create(iomux& io, int fd);


private:
    static void handle(iomux::ptr* ptr);

    unix_client(iomux& io, int fd);
    ~unix_client();

    void handle(void);


private:
    int fd;
    bool alive;

    char buff[1024];

    iomux::ptr iomux_ptr;
    iomux::events iomux_events;
};

} /* namespace lixs */

#endif /* __LIXS_UNIX_CLIENT_HH__ */

