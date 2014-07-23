#ifndef __LIXS_IOMUX_HH__
#define __LIXS_IOMUX_HH__


namespace lixs {

class iok {
public:
    virtual void run(void) = 0;
};

class iokfd {
public:
    struct ioev {
        bool read;
        bool write;

        ioev(bool _read, bool _write)
            : read(_read), write(_write)
        { };
    };

    virtual void handle(const ioev& events) = 0;
};

class iomux {
public:
    virtual void once(iok& k) = 0;

    virtual void add(iokfd& k, int fd, const iokfd::ioev& ev) = 0;
    virtual void set(iokfd& k, int fd, const iokfd::ioev& ev) = 0;
    virtual void remove(int fd) = 0;

    virtual void handle(void) = 0;
};

} /* namespace lixs */

#endif /* __LIXS_IOMUX_HH__ */

