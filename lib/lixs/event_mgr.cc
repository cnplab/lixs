#include <lixs/event_mgr.hh>

#include <iterator>
#include <list>
#include <memory>


lixs::event_mgr::event_mgr(void)
    : active(false)
{
}

lixs::event_mgr::~event_mgr()
{
}

void lixs::event_mgr::run(void)
{
    event_list::iterator it;
    event_list::iterator nit;

    for(it = events.begin(); active && it != events.end(); it = nit) {
        it->operator()();
        nit = std::next(it);
        events.erase(it);
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
    events.push_back(cb);
}

