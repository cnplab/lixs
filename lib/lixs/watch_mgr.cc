#include <lixs/util.hh>
#include <lixs/watch.hh>
#include <lixs/watch_mgr.hh>

#include <memory>


lixs::watch_mgr::watch_mgr(event_mgr& emgr)
    : emgr(emgr)
{
}

lixs::watch_mgr::~watch_mgr()
{
}

void lixs::watch_mgr::add(watch_cb_k& cb)
{
    db[cb.path].path.insert(&cb);

    register_with_parents(cb.path, cb);

    emgr.enqueue_event(std::bind(&watch_mgr::callback, this, cb.path, &cb, cb.path));
}

void lixs::watch_mgr::del(watch_cb_k& cb)
{
    record& rec = db[cb.path];
    rec.path.erase(&cb);
    if (rec.path.size() == 0 && rec.children.size() == 0) {
        db.erase(cb.path);
    }

    unregister_from_parents(cb.path, cb);
}

void lixs::watch_mgr::fire(unsigned int tid, const std::string& path)
{
    if (tid == 0) {
        _fire(path, path);
    } else {
        _tfire(tid, path, path);
    }
}

void lixs::watch_mgr::fire_parents(unsigned int tid, const std::string& path)
{
    if (tid == 0) {
        _fire_parents(path, path);
    } else {
        _tfire_parents(tid, path, path);
    }
}

void lixs::watch_mgr::fire_children(unsigned int tid, const std::string& path)
{
    if (tid == 0) {
        _fire_children(path);
    } else {
        _tfire_children(tid, path);
    }
}

void lixs::watch_mgr::fire_transaction(unsigned int tid)
{
    fire_list::iterator it;

    fire_list& l = tdb[tid];
    for (it = l.begin(); it != l.end(); it++) {
        emgr.enqueue_event(*it);
    }

    tdb.erase(tid);
}

void lixs::watch_mgr::abort_transaction(unsigned int tid)
{
    tdb.erase(tid);
}

void lixs::watch_mgr::callback(const std::string& key, watch_cb_k* cb, const std::string& path)
{
    database::iterator it;

    it = db.find(key);
    if (it != db.end()) {
        record& rec = it->second;

        if (rec.path.find(cb) != rec.path.end()) {
            cb->operator()(path);
        }
    }
}

void lixs::watch_mgr::_fire(const std::string& path, const std::string& fire_path)
{
    watch_set::iterator it;

    record& rec = db[path];
    for (it = rec.path.begin(); it != rec.path.end(); it++) {
        emgr.enqueue_event(std::bind(&watch_mgr::callback, this, path, *it, fire_path));
    }
}

void lixs::watch_mgr::_tfire(unsigned int tid, const std::string& path,
        const std::string& fire_path)
{
    watch_set::iterator it;

    record& rec = db[path];
    fire_list& l = tdb[tid];
    for (it = rec.path.begin(); it != rec.path.end(); it++) {
        l.push_back(std::bind(&watch_mgr::callback, this, path, *it, fire_path));
    }
}

void lixs::watch_mgr::_fire_parents(const std::string& path, const std::string& fire_path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        _fire(parent, fire_path);
        _fire_parents(parent, fire_path);
    }
}

void lixs::watch_mgr::_tfire_parents(unsigned int tid, const std::string& path,
        const std::string& fire_path)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        _tfire(tid, parent, fire_path);
        _tfire_parents(tid, parent, fire_path);
    }
}

void lixs::watch_mgr::_fire_children(const std::string& path)
{
    watch_set::iterator it;

    record& rec = db[path];
    for (it = rec.children.begin(); it != rec.children.end(); it++) {
        emgr.enqueue_event(std::bind(&watch_mgr::callback, this, path, *it, (*it)->path));
    }
}

void lixs::watch_mgr::_tfire_children(unsigned int tid, const std::string& path)
{
    watch_set::iterator it;

    record& rec = db[path];
    fire_list& l = tdb[tid];
    for (it = rec.children.begin(); it != rec.children.end(); it++) {
        l.push_back(std::bind(&watch_mgr::callback, this, path, *it, (*it)->path));
    }
}

void lixs::watch_mgr::register_with_parents(const std::string& path, watch_cb_k& cb)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        db[parent].children.insert(&cb);

        register_with_parents(parent, cb);
    }
}

void lixs::watch_mgr::unregister_from_parents(const std::string& path, watch_cb_k& cb)
{
    std::string name;
    std::string parent;

    if (basename(path, parent, name)) {
        record& rec = db[parent];

        rec.children.erase(&cb);
        if (rec.path.size() == 0 && rec.children.size() == 0) {
            db.erase(parent);
        }

        unregister_from_parents(parent, cb);
    }
}

