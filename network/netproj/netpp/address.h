// Copyright (c) 2017 ASMlover. All rights reserved.
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
#pragma once

#include <cstdint>
#include <string>
#include <Chaos/Base/UnCopyable.h>

struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;

namespace netpp {

class IP;

class Address : private Chaos::UnCopyable {
  struct sockaddr* addr_{};
public:
  explicit Address(const IP& ipv);
  explicit Address(const struct sockaddr_in& addr);
  explicit Address(const struct sockaddr_in6& addr6);
  explicit Address(const IP& ipv, std::uint16_t port, bool loopback = false);
  Address(const IP& ipv, const char* host, std::uint16_t port);
  ~Address(void);
  int get_family(void) const;
  std::string get_host(void) const;
  std::uint16_t get_port(void) const;

  const struct sockaddr* get_address(void) const {
    return addr_;
  }

  struct sockaddr* get_address(void) {
    return addr_;
  }
};

template <typename ProtocolType>
inline ProtocolType get_protocol(const Address& addr) {
  return ProtocolType::get_protocol(addr.get_family());
}

}
