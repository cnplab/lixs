#include <lixs/event_mgr.hh>
#include <lixs/iomux.hh>

#include <list>
#include <memory>


lixs::event_mgr::event_mgr(iomux& io)
    : active(false), io(io)
{
}

lixs::event_mgr::~event_mgr()
{
}

void lixs::event_mgr::run(void)
{
    while (active) {
        fire_events();
        io.handle();
    }
}

void lixs::event_mgr::enable(void)
{
    active = true;
}

void lixs::event_mgr::disable(void)
{
    active = false;
}

void lixs::event_mgr::enqueue_event(std::function<void(void)> cb)
{
    event_list.push_front(cb);
}

void lixs::event_mgr::fire_events(void)
{
    std::list<std::function<void(void)> >::iterator it;

    for(it = event_list.begin(); it != event_list.end(); it++) {
        it->operator()();
    }

    /* FIXME: ensure enqueue_event is not called during this method */

    event_list.clear();
}

