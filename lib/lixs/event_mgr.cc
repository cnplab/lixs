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
    std::list<std::function<void(void)> >::iterator it;
    std::list<std::function<void(void)> >::iterator nit;

    for(it = event_list.begin(); active && it != event_list.end(); it = nit) {
        it->operator()();
        nit = std::next(it);
        event_list.erase(it);
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
    event_list.push_back(cb);
}

