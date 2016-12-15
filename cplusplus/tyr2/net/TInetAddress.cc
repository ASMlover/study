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
#include <assert.h>
#include <string.h>
#include "../basic/TConfig.h"
#include "../basic/TLogging.h"
#include "TSocketSupport.h"
#include "TEndian.h"
#include "TInetAddress.h"

namespace tyr { namespace net {

static const in_addr_t kInAddrAny = INADDR_ANY;
static const in_addr_t kInAddrLoopback = INADDR_LOOPBACK;
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6), "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset must be 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset must be 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset must be 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset must be 2");

InetAddress::InetAddress(uint16_t port, bool loopback_only, bool ipv6) {
  static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset must be 0");
  static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset must be 0");

  if (ipv6) {
    memset(&addr6_, 0, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_addr = loopback_only ? in6addr_loopback : in6addr_any;
    addr6_.sin6_port = host_to_net16(port);
  }
  else {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = host_to_net32(loopback_only ? kInAddrLoopback : kInAddrAny);
    addr_.sin_port = host_to_net16(port);
  }
}

InetAddress::InetAddress(basic::StringPiece host, uint16_t port, bool ipv6) {
  if (ipv6) {
    memset(&addr6_, 0, sizeof(addr6_));
    SocketSupport::kern_from_ip_port(host.data(), port, &addr6_);
  }
  else {
    memset(&addr_, 0, sizeof(addr_));
    SocketSupport::kern_from_ip_port(host.data(), port, &addr_);
  }
}

std::string InetAddress::to_host(void) const {
  char buf[64] = {0};
  SocketSupport::kern_to_ip(buf, sizeof(buf), get_address());
  return buf;
}

std::string InetAddress::to_host_port(void) const {
  char buf[64] = {0};
  SocketSupport::kern_to_ip_port(buf, sizeof(buf), get_address());
  return buf;
}

uint16_t InetAddress::to_port(void) const {
  return net_to_host16(get_port_endian());
}

uint32_t InetAddress::get_host_endian(void) const {
  assert(get_family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

#if defined(TYR_WINDOWS)
static __declspec(thread) char t_resolve_buff[64* 1024];
#else
static __thread char t_resolve_buff[64* 1024];
#endif

bool InetAddress::resolve(basic::StringPiece hostname, InetAddress* result) {
  struct hostent* hentp = nullptr;
  int ret = 0;

#if defined(TYR_WINDOWS)
  hentp = gethostbyname(hostname.data());
  if (nullptr == hentp)
    ret = -1;
#else
  struct hostent hent = {0};
  int herrno = 0;
  ret = gethostbyname_r(hostname.data(), &hent, t_resolve_buff, sizeof(t_resolve_buff), &hentp, &herrno);
#endif

  if (0 == ret && nullptr != hentp) {
    assert(hentp->h_addrtype == AF_INET && hentp->h_length == sizeof(uint32_t));
    result->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(hentp->h_addr);
    return true;
  }
  else {
    if (0 != ret)
      TYRLOG_SYSERR << "InetAddress::resolve";
    return false;
  }
}

}}
