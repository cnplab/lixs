#include <lixs/event_mgr.hh>
#include <lixs/events.hh>


lixs::event_mgr::event_mgr(iomux& io)
    : io(io)
{
}

lixs::event_mgr::~event_mgr()
{
}

void lixs::event_mgr::run(void)
{
    fire_events();
    fire_watches();
    io.handle();
}

void lixs::event_mgr::enqueue_event(std::function<void(void)> cb)
{
    event_list.push_front(cb);
}

void lixs::event_mgr::enqueue_watch(watch_cb_k& cb, const std::string& path)
{
    watch_list[&cb].insert(path);
}

void lixs::event_mgr::dequeue_watch(watch_cb_k& cb)
{
    watch_list.erase(&cb);
}

void lixs::event_mgr::fire_events(void)
{
    std::list<std::function<void(void)> >::iterator it;

    for(it = event_list.begin(); it != event_list.end(); it++) {
        it->operator()();
    }

    event_list.clear();
}

void lixs::event_mgr::fire_watches(void)
{
    std::map<watch_cb_k*, std::set<std::string> >::iterator i;
    std::set<std::string>::iterator j;

    for (i = watch_list.begin(); i != watch_list.end(); i++) {
        for (j = i->second.begin(); j != i->second.end(); j++) {
            i->first->operator()(*j);
        }
    }

    watch_list.clear();
}

