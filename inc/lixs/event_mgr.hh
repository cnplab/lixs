#ifndef __LIXS_EVENT_MGR_HH__
#define __LIXS_EVENT_MGR_HH__

#include <lixs/events.hh>
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

    /* FIXME: cleanup interface
     *
     * The event_mgr should only be aware of event callbacks (nullary function objects) to be
     * called ASAP, and file descriptors to be polled.
     */

    void run(void);

    void io_add(fd_cb_k& cd);
    void io_set(fd_cb_k& cb);
    void io_remove(fd_cb_k& cb);

    void enqueue_event(std::function<void(void)> cb);

    void enqueue_watch(watch_cb_k& cb, const std::string& path);
    void dequeue_watch(watch_cb_k& cb);

private:
    void fire_events(void);
    void fire_watches(void);

private:
    iomux& io;
    std::list<std::function<void(void)> > event_list;
    std::map<watch_cb_k*, std::set<std::string> > watch_list;
};

} /* namespace lixs */

#endif /* __LIXS_EVENT_MGR_HH__ */

