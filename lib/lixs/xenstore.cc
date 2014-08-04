#include <lixs/xenstore.hh>

#include <cstdio>
#include <cstring>
#include <errno.h>


lixs::xenstore::xenstore(store& st, iomux& io)
    : next_tid(1), st(st), io(io), nix(NULL), xen(NULL)
{
    st.ensure(0, "/");
}

lixs::xenstore::~xenstore()
{
}

void lixs::xenstore::run(void)
{
    run_once_ev();
    io.handle();
    fire_watches();
}

int lixs::xenstore::read(unsigned int tid, const char* path, const char** res)
{
    (*res) = st.read(path);

    return (*res) == NULL ? ENOENT : 0;
}

int lixs::xenstore::write(unsigned int tid, char* path, const char* val)
{
    ensure_directory(tid, path);
    st.write(tid, path, val);
    enqueue_watch(path);

    return 0;
}

int lixs::xenstore::mkdir(unsigned int tid, char* path)
{
    ensure_directory(tid, path);
    if (st.ensure(tid, path)) {
        enqueue_watch(path);
    }

    return 0;
}

int lixs::xenstore::rm(unsigned int tid, char* path)
{
    /* FIXME: ensure this deleted all the descendents? */
    st.del(tid, path);
    enqueue_watch(path);

    return 0;
}

int lixs::xenstore::directory(unsigned int tid, const char* path, const char* list[], int* nelems)
{
    (*nelems) = st.get_childs(path, list, *nelems);

    return 0;
}

int lixs::xenstore::transaction_start(unsigned int* tid)
{
    *tid = next_tid++;
    st.branch(*tid);

    return 0;
}

int lixs::xenstore::transaction_end(unsigned int tid, bool commit)
{
    if (commit) {
        return st.merge(tid) ? 0 : EAGAIN;
    } else {
        st.abort(tid);
        return 0;
    }
}

void lixs::xenstore::watch(watch_cb_k& cb)
{
    watch_lst[cb.path].insert(&cb);
}

void lixs::xenstore::unwatch(watch_cb_k& cb)
{
    watch_lst[cb.path].erase(&cb);
}

void lixs::xenstore::get_domain_path(char* domid, char (&buff)[32])
{
    sprintf(buff, "/local/domain/%s", domid);
}

void lixs::xenstore::once(ev_cb_k& cb)
{
    once_lst.push_front(&cb);
}

void lixs::xenstore::add(fd_cb_k& cb)
{
    io.add(cb);
}

void lixs::xenstore::set(fd_cb_k& cb)
{
    io.set(cb);
}

void lixs::xenstore::remove(fd_cb_k& cb)
{
    io.remove(cb);
}

void lixs::xenstore::set_unix_server(unix_server* server)
{
    nix = server;
}

void lixs::xenstore::set_xen_server(xen_server* server)
{
    xen = server;
}

void lixs::xenstore::run_once_ev(void)
{
    for(std::list<ev_cb_k*>::iterator i = once_lst.begin(); i != once_lst.end(); i++) {
        (*i)->operator()();
    }
    once_lst.clear();
}

void lixs::xenstore::fire_watches(void)
{
    std::map<watch_cb_k*, std::set<std::string> >::iterator i;
    std::set<std::string>::iterator j;

    for (i = fire_lst.begin(); i != fire_lst.end(); i++) {
        for (j = i->second.begin(); j != i->second.end();) {
            i->first->operator()(*j);
            i->second.erase(j++);
        }
    }
}

void lixs::xenstore::ensure_directory(int tid, char* path)
{
    unsigned int i;
    unsigned int len;

    i = 1;
    len = strlen(path);

    do {
        i += strcspn(path + i, "/");
        if (i < len) {
            path[i] = '\0';
            st.ensure(tid, path);
            path[i] = '/';
        }

        i++;
    } while(i < len);
}

void lixs::xenstore::enqueue_watch(char* path)
{
    unsigned int i;
    unsigned int len;
    std::set<watch_cb_k*>::iterator it;

    i = 1;
    len = strlen(path);

    do {
        i += strcspn(path + i, "/");
        if (i < len) {
            path[i] = '\0';

            std::set<watch_cb_k*> lst = watch_lst[path];
            for (it = lst.begin(); it != lst.end(); it++) {
                fire_lst[(*it)].insert(path);
            }

            path[i] = '/';
        }

        i++;
    } while(i < len);

    std::set<watch_cb_k*> lst = watch_lst[path];
    for (it = lst.begin(); it != lst.end(); it++) {
        fire_lst[(*it)].insert(path);
    }
}

