#include <lixs/xenstore.hh>

#include <cerrno>
#include <set>
#include <string>


lixs::xenstore::xenstore(store& st, event_mgr& emgr, iomux& io)
    : st(st), wmgr(emgr), dmgr(*this, emgr, io)
{
    bool created;

    st.create(0, "/", created);
}

lixs::xenstore::~xenstore()
{
}

int lixs::xenstore::store_read(unsigned int tid, const std::string& path, std::string& val)
{
    return st.read(tid, path, val);
}

int lixs::xenstore::store_write(unsigned int tid, const std::string& path, const std::string& val)
{
    int ret;

    ret = st.update(tid, path, val);
    if (ret == 0) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
    }

    return ret;
}

int lixs::xenstore::store_mkdir(unsigned int tid, const std::string& path)
{
    int ret;
    bool created;

    ret = st.create(tid, path, created);
    if (ret == 0 && created) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
    }

    return ret;
}

int lixs::xenstore::store_rm(unsigned int tid, const std::string& path)
{
    int ret;

    ret = st.del(tid, path);
    if (ret == 0) {
        wmgr.fire(tid, path);
        wmgr.fire_parents(tid, path);
        wmgr.fire_children(tid, path);
    }

    return ret;
}

int lixs::xenstore::store_dir(unsigned int tid, const std::string& path, std::set<std::string>& res)
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
    int ret;
    bool success;

    if (commit) {
        ret = st.merge(tid, success);

        if (ret == 0) {
            if (success) {
                wmgr.fire_transaction(tid);
            } else {
                wmgr.abort_transaction(tid);
            }

            return success ? 0 : EAGAIN;
        } else {
            return ret;
        }
    } else {
        ret = st.abort(tid);

        if (ret == 0) {
            wmgr.abort_transaction(tid);

            return 0;
        } else {
            return ret;
        }
    }
}

void lixs::xenstore::watch_add(watch_cb_k& cb)
{
    wmgr.add(cb);
}

void lixs::xenstore::watch_del(watch_cb_k& cb)
{
    wmgr.del(cb);
}

void lixs::xenstore::domain_path(domid_t domid, std::string& path)
{
    char numstr[35];
    sprintf(numstr, "/local/domain/%d", domid);

    path = std::string(numstr);
}

void lixs::xenstore::domain_introduce(domid_t domid, unsigned int mfn , evtchn_port_t port)
{
    if (dmgr.create(domid, port, mfn) == 0) {
        wmgr.fire(0, "@introduceDomain");
    }
}

void lixs::xenstore::domain_release(domid_t domid)
{
    if (dmgr.destroy(domid)) {
        wmgr.fire(0, "@releaseDomain");
    }
}

void lixs::xenstore::domain_exists(domid_t domid, bool& exists)
{
    dmgr.exists(domid, exists);
}

