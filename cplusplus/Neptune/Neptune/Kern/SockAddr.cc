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
#include <Chaos/Types.h>
#include <Neptune/Endian.h>
#include <Neptune/Kern/SockAddr.h>

namespace Neptune {

namespace SockAddr {
  std::string to_string(const struct sockaddr* addr) {
    std::string ip = to_string(addr, true);
    const struct sockaddr_in* addr4 = to_v4(addr);
    std::uint16_t port = Neptune::n2h16(addr4->sin_port);

    char buf[64]{};
    std::snprintf(buf, sizeof(buf), "%s:%u", ip.c_str(), port);
    return buf;
  }

  struct sockaddr* cast(struct sockaddr_in* addr) {
    return static_cast<struct sockaddr*>(Chaos::implicit_cast<void*>(addr));
  }

  const struct sockaddr* cast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr*>(Chaos::implicit_cast<const void*>(addr));
  }

  struct sockaddr* cast(struct sockaddr_in6* addr) {
    return static_cast<struct sockaddr*>(Chaos::implicit_cast<void*>(addr));
  }

  const struct sockaddr* cast(const struct sockaddr_in6* addr) {
    return static_cast<const struct sockaddr*>(Chaos::implicit_cast<const void*>(addr));
  }

  const struct sockaddr_in* to_v4(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(Chaos::implicit_cast<const void*>(addr));
  }

  const struct sockaddr_in6* to_v6(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in6*>(Chaos::implicit_cast<const void*>(addr));
  }
}

}
