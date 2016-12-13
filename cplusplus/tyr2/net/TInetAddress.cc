// Copyright (c) 2016 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <string.h>
#include "TSocketSupport.h"
#include "TEndian.h"
#include "TInetAddress.h"

namespace tyr { namespace net {

static const int kInAddrAny = INADDR_ANY;
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in), "InetAddress size must be same as sockaddr_in");

InetAddress::InetAddress(uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = host_to_net32(kInAddrAny);
  addr_.sin_port = host_to_net16(port);
}

InetAddress::InetAddress(const std::string& host, uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  SocketSupport::kern_from_ip_port(host.c_str(), port, &addr_);
}

std::string InetAddress::to_host_port(void) const {
  char buf[32];
  SocketSupport::kern_to_ip_port(buf, sizeof(buf), SocketSupport::kern_sockaddr_cast(&addr_));
  return buf;
}

}}
