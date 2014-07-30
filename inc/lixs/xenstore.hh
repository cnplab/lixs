#ifndef __LIXS_XENSTORE_HH__
#define __LIXS_XENSTORE_HH__

#include <lixs/store.hh>


namespace lixs {

class xenstore {
public:
    xenstore(store& st);
    ~xenstore();

    int read(unsigned int tid, const char* path, const char** res);
    int write(unsigned int tid, char* path, const char* val);
    int mkdir(unsigned int tid, const char* path);
    int rm(unsigned int tid, const char* path);
    int directory(unsigned int tid, const char* path, const char* list[], int* nelems);

    int transaction_start(unsigned int* tid);
    int transaction_end(unsigned int tid, bool commit);

    void get_domain_path(char* domid, char (&buff)[32]);

private:
    static unsigned int next_tid;
    store& st;
};

} /* namespace lixs */

#endif /* __LIXS_XENSTORE_HH__ */

