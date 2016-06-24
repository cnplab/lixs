/*
 * LiXS: Lightweight XenStore
 *
 * Authors: Filipe Manco <filipe.manco@neclab.eu>
 *
 *
 * Copyright (c) 2016, NEC Europe Ltd., NEC Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

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

