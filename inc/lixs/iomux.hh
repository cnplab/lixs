#ifndef __LIXS_IOMUX_HH__
#define __LIXS_IOMUX_HH__


namespace lixs {

class iomux {
public:
    struct events {
        bool read;
        bool write;

        events(bool _read, bool _write)
            : read(_read), write(_write)
        { };
    };

    struct ptr {
        void (*fn)(struct ptr*);
        void* data;

        ptr(void (*_fn)(struct ptr*), void* _data)
            : fn(_fn), data(_data)
        { };
    };

    virtual int add(int fd, const struct events& ev, struct ptr* ptr) = 0;
    virtual int set(int fd, const struct events& ev, struct ptr* ptr) = 0;
    virtual int remove(int fd) = 0;

    virtual int handle(void) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_IOMUX_HH__ */

