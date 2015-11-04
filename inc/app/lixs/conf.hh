#ifndef __APP_LIXS_CONF_HH__
#define __APP_LIXS_CONF_HH__

#include <string>


namespace app {

struct lixs_conf {
public:
    lixs_conf(int argc, char** argv);

public:
    void print_usage();

public:
    bool help;

    bool daemonize;
    bool write_pid_file;
    std::string pid_file;
    bool log_to_file;
    std::string log_file;

    bool xenbus;
    bool virq_dom_exc;
    bool unix_sockets;
    std::string unix_socket_path;
    std::string unix_socket_ro_path;

    bool error;

private:
    std::string cmd;
};

} /* namespace app */

#endif /* __APP_LIXS_CONF_HH__ */

