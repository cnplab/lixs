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
    enqueue(0, cb.path);
}

void lixs::watch_mgr::del(watch_cb_k& cb)
{
    /* FIXME: must remove watches from fire list */
    watch_lst[cb.path].erase(&cb);
    emgr.dequeue_watch(cb);
}

void lixs::watch_mgr::enqueue(unsigned int tid, const std::string& path)
{
    std::set<watch_cb_k*>::iterator it;

    for (it = watch_lst[path].begin(); it != watch_lst[path].end(); it++) {
        if (tid == 0) {
            emgr.enqueue_watch(**it, path);
        } else {
            fire_lst[tid].push_back(std::pair<watch_cb_k&, std::string>(**it, path));
        }
    }
}

void lixs::watch_mgr::enqueue_parents(unsigned int tid, const std::string& path)
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
            if (tid == 0) {
                emgr.enqueue_watch(**it, path);
            } else {
                fire_lst[tid].push_back(std::pair<watch_cb_k&, std::string>(**it, path));
            }
        }
    }
}

void lixs::watch_mgr::enqueue_children(unsigned int tid, const std::string& path)
{
    std::map<std::string, std::set<watch_cb_k*> >::iterator i;
    std::set<watch_cb_k*>::iterator j;

    for (i = watch_lst.begin(); i != watch_lst.end(); i++) {
        if (i->first.find(path) == 0
                && i->first[path.length()] == '/') {
            for (j = i->second.begin(); j != i->second.end(); j++) {
                if (tid == 0) {
                    emgr.enqueue_watch(**j, (*j)->path);
                } else {
                    fire_lst[tid].push_back(std::pair<watch_cb_k&, std::string>(**j, (*j)->path));
                }
            }
        }
    }
}

void lixs::watch_mgr::transaction_commit(unsigned int tid)
{
    std::list<std::pair<watch_cb_k&, std::string> >::iterator it;

    if (fire_lst.find(tid) == fire_lst.end()) {
        return;
    }

    for (it = fire_lst[tid].begin(); it != fire_lst[tid].end(); it++) {
        emgr.enqueue_watch(it->first, it->second);
    }

    fire_lst.erase(tid);
}

void lixs::watch_mgr::transaction_abort(unsigned int tid)
{
    fire_lst.erase(tid);
}

