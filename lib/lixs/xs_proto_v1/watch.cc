#include <lixs/xs_proto_v1/xs_proto.hh>


lixs::xs_proto_v1::watch_cb::watch_cb(xs_proto_base& proto,
        const std::string& path, const std::string& token, bool relative)
    : lixs::watch_cb(path, token), proto(proto), relative(relative)
{
}

void lixs::xs_proto_v1::watch_cb::operator()(const std::string& fire_path)
{
    std::string path;

    if (relative) {
        /* NOTE: Remove dom_path plus '/' */
        path = fire_path.substr(proto.dom_path.length() + 1);
    } else {
        path = fire_path;
    }

    proto.tx_queue.push_back({XS_WATCH_EVENT, 0, 0, { path, token }, true});
    proto.process_tx();
}

