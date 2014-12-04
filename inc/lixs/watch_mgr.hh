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

    void enqueue(unsigned int tid, const std::string& path);
    void enqueue_parents(unsigned int tid, const std::string& path);
    void enqueue_children(unsigned int tid, const std::string& path);

    void transaction_commit(unsigned int tid);
    void transaction_abort(unsigned int tid);

private:
    event_mgr& emgr;

    std::map<std::string, std::set<watch_cb_k*> > watch_lst;
    std::map<int, std::list<std::pair<watch_cb_k&, std::string> > > fire_lst;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

