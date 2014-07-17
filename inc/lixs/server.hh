#ifndef __LIXS_SERVER_HH__
#define __LIXS_SERVER_HH__

#include <lixs/iomux.hh>


namespace lixs {

class server {
public:
    server(iomux& io)
        : io(io)
    { };

    ~server()
    { };

protected:
    iomux& io;
};

} /* namespace lixs */

#endif /* __LIXS_SERVER_HH__ */

