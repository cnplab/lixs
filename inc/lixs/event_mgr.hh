#ifndef __LIXS_EVENT_MGR_HH__
#define __LIXS_EVENT_MGR_HH__

#include <lixs/iomux.hh>

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>


namespace lixs {

class event_mgr {
public:
    event_mgr(iomux& io);
    ~event_mgr();

    void run(void);

    void enqueue_event(std::function<void(void)> cb);

private:
    void fire_events(void);

private:
    iomux& io;
    std::list<std::function<void(void)> > event_list;
};

} /* namespace lixs */

#endif /* __LIXS_EVENT_MGR_HH__ */

