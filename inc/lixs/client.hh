#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/iomux.hh>


namespace lixs {

class client : public iok, public iokfd {
public:
    void run(void);
    void handle(const iokfd::ioev& events);


protected:
    client(iomux& io);
    ~client();

    iomux& io;
};

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

