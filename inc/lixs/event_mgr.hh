#ifndef __LIXS_EVENT_MGR_HH__
#define __LIXS_EVENT_MGR_HH__

#include <lixs/events.hh>
#include <lixs/iomux.hh>

#include <list>
#include <map>
#include <set>
#include <string>


namespace lixs {

class event_mgr {
public:
    event_mgr(iomux& io);
    ~event_mgr();

    /* FIXME: cleanup interface
     *
     * The event_mgr should only be aware of event callbacks (nullary function objects) to be
     * called ASAP, and file descriptors to be polled.
     */

    void run(void);

    void io_add(fd_cb_k& cd);
    void io_set(fd_cb_k& cb);
    void io_remove(fd_cb_k& cb);

    void enqueue_event(ev_cb_k& cb);
    void enqueue_watch(watch_cb_k& cb, const std::string& path);

private:
    iomux& io;
    std::list<ev_cb_k*> event_list;
    std::map<watch_cb_k*, std::set<std::string> > watch_list;

    void fire_events(void);
    void fire_watches(void);
};

} /* namespace lixs */

#endif /* __LIXS_EVENT_MGR_HH__ */

