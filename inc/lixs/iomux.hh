#ifndef __LIXS_IOMUX_HH__
#define __LIXS_IOMUX_HH__

#include <lixs/events.hh>


namespace lixs {

class iomux {
public:
    virtual void add(fd_cb_k& cb) = 0;
    virtual void set(fd_cb_k& cb) = 0;
    virtual void remove(fd_cb_k& cb) = 0;

    virtual void handle(void) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_IOMUX_HH__ */

