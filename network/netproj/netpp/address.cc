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
#include <Chaos/Base/Platform.h>
#if defined(CHAOS_WINDOWS)
# include <WS2tcpip.h>
# if !defined(_WINDOWS_)
#   include <WinSock2.h>
# endif
#else
# include <arpa/inet.h>
#endif
#include "compatible_endian.h"
#include "primitive.h"
#include "protocol.h"
#include "address.h"

namespace netpp {

Address::Address(const IP& ipv)
  : addr_(ipv.family() == AF_INET6
      ? reinterpret_cast<struct sockaddr*>(new sockaddr_in6())
      : reinterpret_cast<struct sockaddr*>(new sockaddr_in())) {
}

Address::Address(const struct sockaddr_in& addr)
  : addr_(reinterpret_cast<struct sockaddr*>(new sockaddr_in(addr))) {
}

Address::Address(const struct sockaddr_in6& addr6)
  : addr_(reinterpret_cast<struct sockaddr*>(new sockaddr_in6(addr6))) {
}

Address::Address(const IP& ipv, std::uint16_t port, bool loopback) {
  if (ipv.family() == AF_INET6) {
    auto* addr6 = new sockaddr_in6();
    addr6->sin6_family = AF_INET6;
    addr6->sin6_addr = loopback ? in6addr_loopback : in6addr_any;
    addr6->sin6_port = netpp::h2n16(port);
    addr_ = reinterpret_cast<struct sockaddr*>(addr6);
  }
  else {
    auto* addr4 = new sockaddr_in();
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr =
      netpp::h2n32(loopback ? INADDR_LOOPBACK : INADDR_ANY);
    addr4->sin_port = netpp::h2n16(port);
    addr_ = reinterpret_cast<struct sockaddr*>(addr4);
  }
}

Address::Address(const IP& ipv, const char* host, std::uint16_t port) {
  if (ipv.family() == AF_INET6) {
    auto* addr6 = new sockaddr_in6();
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = netpp::h2n16(port);
    netpp::inet_pton(AF_INET6, host, &addr6->sin6_addr);
    addr_ = reinterpret_cast<struct sockaddr*>(addr6);
  }
  else {
    auto* addr4 = new sockaddr_in();
    addr4->sin_family = AF_INET;
    addr4->sin_port = netpp::h2n16(port);
    netpp::inet_pton(AF_INET, host, &addr4->sin_addr);
    addr_ = reinterpret_cast<struct sockaddr*>(addr4);
  }
}

Address::~Address(void) {
  if (addr_->sa_family == AF_INET6)
    delete reinterpret_cast<struct sockaddr_in6*>(addr_);
  else
    delete reinterpret_cast<struct sockaddr_in*>(addr_);
}

int Address::get_family(void) const {
  return addr_->sa_family;
}

std::string Address::get_host(void) const {
  char buf[64]{};
  if (addr_->sa_family == AF_INET6) {
    auto* addr6 = reinterpret_cast<struct sockaddr_in6*>(addr_);
    netpp::inet_ntop(AF_INET6, &addr6->sin6_addr, sizeof(buf), buf);
  }
  else {
    auto* addr4 = reinterpret_cast<struct sockaddr_in*>(addr_);
    netpp::inet_ntop(AF_INET, &addr4->sin_addr, sizeof(buf), buf);
  }

  return std::string(buf);
}

std::uint16_t Address::get_port(void) const {
  if (addr_->sa_family == AF_INET6) {
    auto* addr6 = reinterpret_cast<struct sockaddr_in6*>(addr_);
    return addr6->sin6_port;
  }
  else {
    auto* addr4 = reinterpret_cast<struct sockaddr_in*>(addr_);
    return addr4->sin_port;
  }
}

}
