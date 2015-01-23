#include <lixs/xenstore.hh>
#include <lixs/domain_mgr.hh>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <set>


lixs::xenstore::xenstore(store& st, event_mgr& emgr)
    : st(st), emgr(emgr), wmgr(emgr), dmgr(*this, emgr)
{
    bool created;

    st.create(0, "/", created);
}

lixs::xenstore::~xenstore()
{
}

int lixs::xenstore::read(unsigned int tid, std::string path, std::string& res)
{
    return st.read(tid, path, res);
}

int lixs::xenstore::write(unsigned int tid, char* path, const char* val)
{
    st.update(tid, path, val);
    wmgr.fire(tid, path);
    wmgr.fire_parents(tid, path);

    return 0;
}

int lixs::xenstore::mkdir(unsigned int tid, char* path)
{
    bool created;

    st.create(tid, path, created);

    if (created) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
    }

    return 0;
}

int lixs::xenstore::rm(unsigned int tid, char* path)
{
    st.del(tid, path);
    wmgr.fire(tid, path);
    wmgr.fire_parents(tid, path);
    wmgr.fire_children(tid, path);

    return 0;
}

int lixs::xenstore::directory(unsigned int tid, const char* path, std::set<std::string>& res)
{
    return st.get_children(tid, path, res);
}

int lixs::xenstore::transaction_start(unsigned int* tid)
{
    st.branch(*tid);

    return 0;
}

int lixs::xenstore::transaction_end(unsigned int tid, bool commit)
{
    if (commit) {
        bool success;
        st.merge(tid, success);

        if (success) {
            wmgr.fire_transaction(tid);
        } else {
            wmgr.abort_transaction(tid);
        }

        return success ? 0 : EAGAIN;
    } else {
        st.abort(tid);
        wmgr.abort_transaction(tid);
        return 0;
    }
}

void lixs::xenstore::watch(watch_cb_k& cb)
{
    wmgr.add(cb);
}

void lixs::xenstore::unwatch(watch_cb_k& cb)
{
    wmgr.del(cb);
}

void lixs::xenstore::get_domain_path(int domid, char *buff)
{
    sprintf(buff, "/local/domain/%d", domid);
}

void lixs::xenstore::get_domain_path(char* domid, char (&buff)[32])
{
    sprintf(buff, "/local/domain/%s", domid);
}

void lixs::xenstore::introduce_domain(int domid, int mfn , int port)
{
    dmgr.create_domain(domid, port, mfn);
    wmgr.fire(0, "@introduceDomain");
}

void lixs::xenstore::release_domain(int domid)
{
    dmgr.destroy_domain(domid);
    wmgr.fire(0, "@releaseDomain");
}

void lixs::xenstore::exists_domain(int domid, bool& exists)
{
    dmgr.exists_domain(domid, exists);
}

