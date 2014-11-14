#ifndef __LIXS_WATCH_MGR_HH__
#define __LIXS_WATCH_MGR_HH__

#include <lixs/events.hh>
#include <lixs/event_mgr.hh>

#include <list>
#include <map>
#include <set>
#include <string>


namespace lixs {

class watch_mgr
{
public:
    watch_mgr(event_mgr& emgr);
    ~watch_mgr();

    /* FIXME: handle transactions */

    void add(watch_cb_k& cb);
    void del(watch_cb_k& cb);

    void enqueue(const std::string& path);
    void enqueue_parents(const std::string& path);
    void enqueue_children(const std::string& path);

private:
    event_mgr& emgr;

    std::map<std::string, std::set<watch_cb_k*> > watch_lst;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

