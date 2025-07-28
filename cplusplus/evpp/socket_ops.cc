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
#include <iostream>
#include "socket_ops.hh"

namespace ev {

static const str_t kEmptyString;

str_t strerror(int e) noexcept {
#if defined(EV_WINDOWS)
  LPVOID buf = nullptr;
  ::FromMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, nullptr);

  if (nullptr != buf) {
    str_t s = (char*)buf;
    ::LocalFree(buf);
    return s;
  }
#elif defined(EV_DARWIN)
  char buf[2048] = {};
  int rc = ::strerror_r(e, buf, sizeof(buf) - 1);
  if (0 == rc)
    return str_t(buf);
#else
  char buf[2048] = {};
# if (_POSIX_C_SOURC > 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
  int rc = ::strerror_r(e, buf, sizeof(buf) - 1);
  if (0 == rc)
    return str_t(buf);
# else
  const char* s = ::strerror_r(e, buf, sizeof(buf) - 1);
  if (nullptr != s)
    return str_t(s);
# endif
#endif
  return kEmptyString;
}

namespace sock {
  socket_t create_nonblocking_socket() noexcept {
    int serrno = 0;

    socket_t sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == -1) {
      serrno = errno;
      std::cerr << "socket error: " << ev::strerror(serrno) << std::endl;
      return INVALID_SOCKET;
    }

    // TODO: need libevent dependency

    return sockfd;
  }

  // socket_t create_udp_server(int port) noexcept;
  // void set_keep_alive(socket_t sockfd, bool flag) noexcept;
  // void set_reuse_addr(socket_t sockfd) noexcept;
  // void set_reuse_port(socket_t sockfd) noexcept;
  // void set_tcp_nodelay(socket_t sockfd, bool flag) noexcept;
  // void set_timeout(socket_t sockfd, u32_t timeout_ms) noexcept;
  // void set_timeout(socket_t sockfd, const Duration& timeout) noexcept;
  // str_t to_addr(const struct sockaddr_storage* ss) noexcept;
  // str_t to_addr(const struct sockaddr* ss) noexcept;
  // str_t to_addr(const struct sockaddr_in* ss) noexcept;
  // str_t to_ip(const struct sockaddr* ss) noexcept;

  // bool parse_from_addr(const char* addr, struct sockaddr_storage& ss) noexcept;
}

}
