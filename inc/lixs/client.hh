#ifndef __LIXS_CLIENT_HH__
#define __LIXS_CLIENT_HH__

#include <lixs/log/logger.hh>

#include <cstdio>
#include <string>
#include <utility>


namespace lixs {

template < typename PROTOCOL >
class client : public PROTOCOL {
public:
    template < typename... ARGS >
    client(const std::string& cid, log::logger& log, ARGS&&... args);
    virtual ~client();

private:
    std::string cid(void);

private:
    std::string client_id;
    log::logger& log;
};


template < typename PROTOCOL >
template < typename... ARGS >
client<PROTOCOL>::client(const std::string& cid, log::logger& log, ARGS&&... args)
    : PROTOCOL(std::forward<ARGS>(args)...), client_id(cid), log(log)
{
    log::LOG<log::level::INFO>::logf(log, "[%4s] New client registered", client_id.c_str());
}

template < typename PROTOCOL >
client<PROTOCOL>::~client()
{
    log::LOG<log::level::INFO>::logf(log, "[%4s] Client connection closed", client_id.c_str());
}

template < typename PROTOCOL >
std::string client<PROTOCOL>::cid(void)
{
    return client_id;
}

} /* namespace lixs */

#endif /* __LIXS_CLIENT_HH__ */

