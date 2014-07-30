#include <lixs/xenstore.hh>

#include <cstdio>
#include <cstring>
#include <errno.h>


unsigned int lixs::xenstore::next_tid = 1;

lixs::xenstore::xenstore(store& st)
    : st(st)
{
    st.ensure("/");
}

lixs::xenstore::~xenstore()
{
}

int lixs::xenstore::read(unsigned int tid, const char* path, const char** res)
{
    (*res) = st.read(path);

    return (*res) == NULL ? ENOENT : 0;
}

int lixs::xenstore::write(unsigned int tid, char* path, const char* val)
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

    st.write(tid, path, val);

    return 0;
}

int lixs::xenstore::mkdir(unsigned int tid, const char* path)
{
    st.ensure(tid, path);

    return 0;
}

int lixs::xenstore::rm(unsigned int tid, const char* path)
{
    /* FIXME: ensure this deleted all the descendents? */
    st.del(tid, path);

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

void lixs::xenstore::get_domain_path(char* domid, char (&buff)[32])
{
    sprintf(buff, "/local/domain/%s", domid);
}

