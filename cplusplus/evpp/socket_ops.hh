// Copyright (c) 2025 ASMlover. All rights reserved.
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

#include <string.h>
#include "net_header.hh"

namespace ev {

class Duration;

str_t strerror(int e) noexcept;

namespace sock {
  socket_t create_nonblocking_socket() noexcept;
  socket_t create_udp_server(int port) noexcept;
  void set_keep_alive(socket_t sockfd, bool flag) noexcept;
  void set_reuse_addr(socket_t sockfd) noexcept;
  void set_reuse_port(socket_t sockfd) noexcept;
  void set_tcp_nodelay(socket_t sockfd, bool flag) noexcept;
  void set_timeout(socket_t sockfd, u32_t timeout_ms) noexcept;
  void set_timeout(socket_t sockfd, const Duration& timeout) noexcept;
  str_t to_addr(const struct sockaddr_storage* ss) noexcept;
  str_t to_addr(const struct sockaddr* ss) noexcept;
  str_t to_addr(const struct sockaddr_in* ss) noexcept;
  str_t to_ip(const struct sockaddr* ss) noexcept;

  bool parse_from_addr(const char* addr, struct sockaddr_storage& ss) noexcept;

  inline struct sockaddr_storage parse_from_addr(const char* addr) noexcept {
    struct sockaddr_storage ss;
    bool rc = parse_from_addr(addr, ss);
    if (rc) {
      return ss;
    }
    else {
      memset(&ss, 0, sizeof(ss));
      return ss;
    }
  }

  bool split_addr(const char* addr, str_t& host, int& port) noexcept;
  struct sockaddr_storage get_local_addr(socket_t sockfd) noexcept;

  inline bool is_zero_addr(const struct sockaddr_storage* ss) noexcept {
    const char* p = reinterpret_cast<const char*>(ss);
    for (sz_t i = 0; i < sizeof(*ss); ++i) {
      if (0 != p[i])
        return false;
    }
    return true;
  }

  inline const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) noexcept {
    return static_cast<const struct sockaddr*>(ev::implicit_cast<const void*>(addr));
  }

  inline struct sockaddr* sockaddr_cast(struct sockaddr_in* addr) noexcept {
    return static_cast<struct sockaddr*>(ev::implicit_cast<void*>(addr));
  }

  inline struct sockaddr* sockaddr_cast(struct sockaddr_storage* addr) noexcept {
    return static_cast<struct sockaddr*>(ev::implicit_cast<void*>(addr));
  }

  inline const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) noexcept {
    return static_cast<const struct sockaddr_in*>(ev::implicit_cast<const void*>(addr));
  }

  inline struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr) noexcept {
    return static_cast<struct sockaddr_in*>(ev::implicit_cast<void*>(addr));
  }

  inline struct sockaddr_in* sockaddr_in_cast(struct sockaddr_storage* addr) noexcept {
    return static_cast<struct sockaddr_in*>(ev::implicit_cast<void*>(addr));
  }

  inline const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr_storage* addr) noexcept {
    return static_cast<const struct sockaddr_in*>(ev::implicit_cast<const void*>(addr));
  }

  inline struct sockaddr_in6* sockaddr_in6_cast(struct sockaddr_storage* addr) noexcept {
    return static_cast<struct sockaddr_in6*>(ev::implicit_cast<void*>(addr));
  }

  inline const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr_storage* addr) noexcept {
    return static_cast<const struct sockaddr_in6*>(ev::implicit_cast<const void*>(addr));
  }

  inline const struct sockaddr_storage* sockaddr_storage_cast(const struct sockaddr* addr) noexcept {
    return static_cast<const struct sockaddr_storage*>(ev::implicit_cast<const void*>(addr));
  }

  inline const struct sockaddr_storage* sockaddr_storage_cast(const struct sockaddr_in* addr) noexcept {
    return static_cast<const struct sockaddr_storage*>(ev::implicit_cast<const void*>(addr));
  }

  inline const struct sockaddr_storage* sockaddr_storage_cast(const struct sockaddr_in6* addr) noexcept {
    return static_cast<const struct sockaddr_storage*>(ev::implicit_cast<const void*>(addr));
  }
}

}
