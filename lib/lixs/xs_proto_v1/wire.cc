#include <lixs/xs_proto_v1/xs_proto.hh>

#include <cstdio>


lixs::xs_proto_v1::wire::wire(std::string dom_path)
    : abs_path(buff), body(buff + dom_path.length() + 1)
{
    std::sprintf(abs_path, "%s/", dom_path.c_str());
}

