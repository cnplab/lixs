#ifndef __LIXS_WATCH_MGR_HH__
#define __LIXS_WATCH_MGR_HH__

#include <lixs/events.hh>
#include <lixs/event_mgr.hh>

#include <list>
#include <map>
#include <set>
#include <string>


namespace lixs {

class watch_mgr {
public:
    watch_mgr(event_mgr& emgr);
    ~watch_mgr();

    void add(watch_cb_k& cb);
    void del(watch_cb_k& cb);

    void fire(unsigned int tid, const std::string& path);
    void fire_parents(unsigned int tid, const std::string& path);
    void fire_children(unsigned int tid, const std::string& path);

    void fire_transaction(unsigned int tid);
    void abort_transaction(unsigned int tid);

private:
    typedef std::set<watch_cb_k*> watch_set;

    struct record {
        watch_set path;
        watch_set children;
    };

    typedef std::map<std::string, record> database;

    typedef std::list<std::pair<watch_cb_k&, std::string> > fire_list;

    typedef std::map<unsigned int, fire_list> transaction_database;

private:
    void _fire(const std::string& path, const std::string& fire_path);
    void _tfire(unsigned int tid, const std::string& path, const std::string& fire_path);
    void _fire_parents(const std::string& path, const std::string& fire_path);
    void _tfire_parents(unsigned int tid, const std::string& path, const std::string& fire_path);
    void _fire_children(const std::string& path);
    void _tfire_children(unsigned int tid, const std::string& path);
    void register_with_parents(const std::string& path, watch_cb_k& cb);
    void unregister_from_parents(const std::string& path, watch_cb_k& cb);

private:
    event_mgr& emgr;

    database db;
    transaction_database tdb;
};

} /* namespace lixs */

#endif /* __LIXS_WATCH_MGR_HH__ */

