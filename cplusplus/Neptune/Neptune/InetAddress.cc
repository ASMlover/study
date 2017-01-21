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
#include <cstddef>
#include <cstring>
#include <Chaos/Platform.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Endian.h>
#include <Neptune/InetAddress.h>

namespace Neptune {

static const in_addr_t kInAddrAny = INADDR_ANY;
static const in_addr_t kInAddrLoopback = INADDR_LOOPBACK;
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6), "InetAddress is same size as sockaddr_in6");
#if defined(CHAOS_DARWIN)
  static_assert(offsetof(sockaddr_in, sin_family) == 1, "sin_family offset must be 1");
  static_assert(offsetof(sockaddr_in6, sin6_family) == 1, "sin6_family offset must be 1");
#else
  static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset must be 0");
  static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset must be 0");
#endif
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset must be 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset must be 2");

InetAddress::InetAddress(std::uint16_t port, bool loopback_only, bool ipv6) {
  static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset must be 0");
  static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset must be 0");
  if (ipv6) {
    memset(&addr6_, 0, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_addr = loopback_only ? in6addr_loopback : in6addr_any;
    addr6_.sin6_port = Neptune::h2n16(port);
  }
  else {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = Neptune::h2n32(loopback_only ? kInAddrLoopback : kInAddrAny);
    addr_.sin_port = Neptune::h2n16(port);
  }
}

InetAddress::InetAddress(Chaos::StringPiece ip, std::uint16_t port, bool ipv6) {
  if (ipv6) {
    memset(&addr6_, 0, sizeof(addr6_));
    NetOps::addr::get_address(ip.data(), port, &addr6_);
  }
  else {
    memset(&addr_, 0, sizeof(addr_));
    NetOps::addr::get_address(ip.data(), port, &addr_);
  }
}

std::string InetAddress::get_host(void) const {
  return NetOps::addr::to_string(get_address(), true);
}

std::string InetAddress::get_host_port(void) const {
  return NetOps::addr::to_string(get_address());
}

std::uint16_t InetAddress::get_port(void) const {
  return Neptune::n2h16(get_port_endian());
}

std::uint32_t InetAddress::get_host_endian(void) const {
  CHAOS_CHECK(get_family() == AF_INET, "sa_family should be AF_INET");
  return addr_.sin_addr.s_addr;
}

#if defined(CHAOS_LINUX)
  static __chaos_tl char t_resolve_buff[64 * 1024];
#endif
bool InetAddress::resolve(Chaos::StringPiece hostname, InetAddress& result) {
  struct hostent* hentp{};
  int rc{};

#if defined(CHAOS_LINUX)
  struct hostent hent{};
  int herrno = 0;
  rc = gethostbyname_r(hostname.data(), &hent, t_resolve_buff, sizeof(t_resolve_buff), &hentp, &herrno);
#else
  hentp = gethostbyname(hostname.data());
  if (!hentp)
    rc = 1;
#endif

  if (rc == 0 && !hentp) {
    CHAOS_CHECK(hentp->h_addrtype == AF_INET && hentp->h_length == sizeof(std::uint32_t),
        "addrtype should be AF_INET and length should be sizeof(uint32_t)");
    result.addr_.sin_addr = *reinterpret_cast<struct in_addr*>(hentp->h_addr);
    return true;
  }
  else {
    if (rc != 0)
      CHAOSLOG_SYSERR << "InetAddress::resolve";
    return false;
  }
}

}
