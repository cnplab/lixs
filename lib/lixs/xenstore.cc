#include <lixs/xenstore.hh>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <list>


lixs::xenstore::xenstore(store& st, event_mgr& emgr)
    : st(st), emgr(emgr), wmgr(emgr), xen(NULL)
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
    wmgr.enqueue(path);
    wmgr.enqueue_parents(path);

    return 0;
}

int lixs::xenstore::mkdir(unsigned int tid, char* path)
{
    bool created;

    st.create(tid, path, created);

    if (created) {
        wmgr.enqueue(path);
        wmgr.enqueue_parents(path);
    }

    return 0;
}

int lixs::xenstore::rm(unsigned int tid, char* path)
{
    /* FIXME: delete all the descendents */

    st.del(tid, path);
    wmgr.enqueue(path);
    wmgr.enqueue_parents(path);
    wmgr.enqueue_children(path);

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
        return success ? 0 : EAGAIN;
    } else {
        st.abort(tid);
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
    xen->create_domain(domid, port);
    wmgr.enqueue((char*)"@introduceDomain");
}

void lixs::xenstore::release_domain(int domid)
{
    xen->destroy_domain(domid);
    wmgr.enqueue((char*)"@releaseDomain");
}

void lixs::xenstore::set_xen_server(xen_server* server)
{
    xen = server;
}

