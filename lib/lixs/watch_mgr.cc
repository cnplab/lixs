#include <lixs/events.hh>
#include <lixs/watch_mgr.hh>

#include <map>
#include <set>

lixs::watch_mgr::watch_mgr(event_mgr& emgr)
    : emgr(emgr)
{
}

lixs::watch_mgr::~watch_mgr()
{
}

void lixs::watch_mgr::add(watch_cb_k& cb)
{
    watch_lst[cb.path].insert(&cb);
    enqueue(cb.path);
}

void lixs::watch_mgr::del(watch_cb_k& cb)
{
    /* FIXME: must remove watches from fire list */
    watch_lst[cb.path].erase(&cb);
}

void lixs::watch_mgr::enqueue(const std::string& path)
{
    std::set<watch_cb_k*>::iterator it;

    for (it = watch_lst[path].begin(); it != watch_lst[path].end(); it++) {
        emgr.enqueue_watch(**it, path);
    }
}

void lixs::watch_mgr::enqueue_parents(const std::string& path)
{
    std::set<watch_cb_k*>::iterator it;
    std::string parent = path;
    size_t pos;

    for ( ; ; ) {
        pos = parent.rfind('/');
        if (pos == std::string::npos) {
            break;
        }

        parent = parent.substr(0, pos);

        for (it = watch_lst[parent].begin(); it != watch_lst[parent].end(); it++) {
            emgr.enqueue_watch(**it, path);
        }
    }
}

void lixs::watch_mgr::enqueue_children(const std::string& path)
{
    std::map<std::string, std::set<watch_cb_k*> >::iterator i;
    std::set<watch_cb_k*>::iterator j;

    for (i = watch_lst.begin(); i != watch_lst.end(); i++) {
        if (i->first.find(path) == 0
                && i->first[path.length()] == '/') {
            for (j = i->second.begin(); j != i->second.end(); j++) {
                emgr.enqueue_watch(**j, (*j)->path);
            }
        }
    }
}

