#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/iomux.hh>


namespace lixs {

class client {
public:
    client(iomux& io)
        : io(io)
    { };

    ~client()
    { };

protected:
    iomux& io;
};

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

