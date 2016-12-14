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
#ifndef __TYR_NET_INETADDRESS_HEADER_H__
#define __TYR_NET_INETADDRESS_HEADER_H__

#include <string>
#include "../basic/TTypes.h"
#include "../basic/TStringPiece.h"
#include "TSocketSupport.h"

namespace tyr { namespace net {

class InetAddress : private basic::UnCopyable {
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
public:
  explicit InetAddress(const struct sockaddr_in& addr)
    : addr_(addr) {
  }

  explicit InetAddress(const struct sockaddr_in6& addr6)
    : addr6_(addr6) {
  }

  explicit InetAddress(uint16_t port = 0, bool loopback_only = false, bool ipv6 = false);
  InetAddress(basic::StringPiece host, uint16_t port, bool ipv6 = false);
  std::string to_host(void) const;
  std::string to_host_port(void) const;
  uint16_t to_port(void) const;
  uint32_t get_host_endian(void) const;

  uint16_t get_port_endian(void) const {
    return addr_.sin_port;
  }

  sa_family_t get_family(void) const {
    return addr_.sin_family;
  }

  const struct sockaddr* get_address(void) const {
    return SocketSupport::kern_sockaddr_cast(&addr6_);
  }

  void set_address(const struct sockaddr_in6& addr6) {
    addr6_ = addr6;
  }

  static bool resolve(basic::StringPiece hostname, InetAddress* result);
};

}}

#endif // __TYR_NET_INETADDRESS_HEADER_H__
