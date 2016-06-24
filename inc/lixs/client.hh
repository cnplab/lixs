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

