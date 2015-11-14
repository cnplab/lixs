#ifndef __LIXS_EVENT_MGR_HH__
#define __LIXS_EVENT_MGR_HH__

#include <list>
#include <memory>


namespace lixs {

class event_mgr {
public:
    event_mgr(void);
    ~event_mgr();

    void run(void);

    void enable(void);
    void disable(void);

    void enqueue_event(std::function<void(void)> cb);

private:
    bool active;

    std::list<std::function<void(void)> > event_list;
};

} /* namespace lixs */

#endif /* __LIXS_EVENT_MGR_HH__ */

