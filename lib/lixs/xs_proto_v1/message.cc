#include <lixs/xs_proto_v1/xs_proto.hh>


lixs::xs_proto_v1::message::message(uint32_t type, uint32_t req_id, uint32_t tx_id,
        std::list<std::string> body, bool terminator)
    : type(type), req_id(req_id), tx_id(tx_id), body(body), terminator(terminator)
{
}

