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

#ifndef __LIXS_OS_LINUX_DOM_EXC_HH__
#define __LIXS_OS_LINUX_DOM_EXC_HH__

#include <lixs/domain_mgr.hh>
#include <lixs/iomux.hh>
#include <lixs/xenstore.hh>

#include <cerrno>
#include <stdexcept>

extern "C" {
#include <xenctrl.h>
}


namespace lixs {
namespace os_linux {

class dom_exc_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class dom_exc {
public:
    dom_exc(xenstore& xs, domain_mgr& dmgr, iomux& io);
    virtual ~dom_exc();

    void callback(bool read, bool write, bool error);

private:
    xenstore& xs;
    domain_mgr& dmgr;
    iomux& io;

    int fd;

    bool alive;

    xc_interface* xc_handle;
    xc_evtchn *xce_handle;
    evtchn_port_t virq_port;
};

} /* namespace os_linux */
} /* namespace lixs */

#endif /* __LIXS_OS_LINUX_DOM_EXC_HH__ */

