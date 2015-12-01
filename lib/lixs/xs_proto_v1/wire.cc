#include <lixs/xs_proto_v1/xs_proto.hh>

#include <cstdio>
#include <sstream>


lixs::xs_proto_v1::wire::wire(std::string dom_path)
    : abs_path(buff), body(buff + dom_path.length() + 1)
{
    std::sprintf(abs_path, "%s/", dom_path.c_str());
}

void lixs::xs_proto_v1::wire::sanitize_input(void)
{
    body[hdr.len] = '\0';
}

lixs::xs_proto_v1::wire::operator std::string () const
{
    unsigned int i;
    std::string sep;
    std::ostringstream stream;

    stream << "{ ";
    stream << "type = " << hdr.type << ", ";
    stream << "req_id = " << hdr.req_id << ", ";
    stream << "tx_id = " << hdr.tx_id << ", ";
    stream << "len = " << hdr.len << ", ";
    stream << "msg = ";

    sep = "\"";
    for (i = 0; i < hdr.len; i += strlen(body + i) + 1) {
        stream << sep << (body + i);
        sep = " ";
    }

    if (i == 0) {
        stream << "\"";
    }

    if (i > 0 && i == hdr.len) {
        stream << " ";
    }

    stream << "\" }";

    return stream.str();
}

