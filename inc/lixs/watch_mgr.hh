#ifndef __LIXS_WATCH_MGR_HH__
#define __LIXS_WATCH_MGR_HH__

#include <lixs/events.hh>

#include <list>
#include <map>
#include <set>
#include <string>


namespace lixs {

class watch_mgr
{
public:
    /* FIXME: handle transactions */

    void add(watch_cb_k& cb);
    void del(watch_cb_k& cb);

    void enqueue(const std::string& path);
    void enqueue_parents(const std::string& path);
    void enqueue_children(const std::string& path);

    void fire(void);

private:
    std::map<std::string, std::set<watch_cb_k*> > watch_lst;
    std::map<watch_cb_k*, std::set<std::string> > fire_lst;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

