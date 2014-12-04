#include <lixs/xenstore.hh>
#include <lixs/domain_mgr.hh>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <list>


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
    wmgr.enqueue(tid, path);
    wmgr.enqueue_parents(tid, path);

    return 0;
}

int lixs::xenstore::mkdir(unsigned int tid, char* path)
{
    bool created;

    st.create(tid, path, created);

    if (created) {
        wmgr.enqueue(tid, path);
        wmgr.enqueue_parents(tid, path);
    }

    return 0;
}

int lixs::xenstore::rm(unsigned int tid, char* path)
{
    /* FIXME: delete all the descendents */

    st.del(tid, path);
    wmgr.enqueue(tid, path);
    wmgr.enqueue_parents(tid, path);
    wmgr.enqueue_children(tid, path);

    return 0;
}

int lixs::xenstore::directory(unsigned int tid, const char* path, std::list<std::string>& list)
{
    st.get_children(path, list);

    return 0;
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
            wmgr.transaction_commit(tid);
        } else {
            wmgr.transaction_abort(tid);
        }

        return success ? 0 : EAGAIN;
    } else {
        st.abort(tid);
        wmgr.transaction_abort(tid);
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
    dmgr.create_domain(domid, port);
    wmgr.enqueue(0, (char*)"@introduceDomain");
}

void lixs::xenstore::release_domain(int domid)
{
    dmgr.destroy_domain(domid);
    wmgr.enqueue(0, (char*)"@releaseDomain");
}

void lixs::xenstore::exists_domain(int domid, bool& exists)
{
    dmgr.exists_domain(domid, exists);
}

