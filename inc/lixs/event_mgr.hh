#ifndef __LIXS_EVENT_MGR_HH__
#define __LIXS_EVENT_MGR_HH__

#include <list>
#include <memory>


namespace lixs {

typedef std::function<void(void)> ev_cb;

class event_mgr {
public:
    event_mgr(void);
    ~event_mgr();

    void run(void);

    void enable(void);
    void disable(void);

    void enqueue_event(ev_cb);

private:
    typedef std::list<ev_cb> event_list;

private:
    bool active;

    event_list events;
};

} /* namespace lixs */

#endif /* __LIXS_EVENT_MGR_HH__ */

