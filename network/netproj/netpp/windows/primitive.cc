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
#include <WS2tcpip.h>
#if !defined(_WINDOWS_)
#  include <WinSock2.h>
#endif
#include <Chaos/IO/ColorIO.h>
#include "../primitive.h"

using socklen_t = int;

namespace netpp {

void startup(void) {
  WSADATA wd{};

  if (::WSAStartup(MAKEWORD(2, 2), &wd) != 0) {
    namespace cc = Chaos::ColorIO;
    cc::fprintf(stderr, cc::ColorType::COLORTYPE_FG_RED,
        "netpp::startup - WinSock2 library startup failed\n");
  }
}

void cleanup(void) {
  ::WSACleanup();
}

namespace socket {
  int close(socket_t sockfd, std::error_code& ec) {
    int r{};
    if (sockfd != kInvalidSocket) {
      clear_last_errno();
      r = error_wrapper(::closesocket(sockfd), ec);
    }

    if (r == 0)
      ec = std::error_code();
    return r;
  }

  int read(socket_t sockfd, std::size_t len, void* buf, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = std::make_error_code(std::errc::bad_file_descriptor);
      return kSocketError;
    }

    clear_last_errno();
    auto nread = error_wrapper(::recv(sockfd,
          static_cast<char*>(buf), static_cast<int>(len), 0), ec);
    if (nread >= 0)
      ec = std::error_code();
    return nread;
  }

  int write(socket_t sockfd,
      const void* buf, std::size_t len, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = std::make_error_code(std::errc::bad_file_descriptor);
      return kSocketError;
    }

    clear_last_errno();
    auto nwrote = error_wrapper(::send(sockfd,
          static_cast<const char*>(buf), static_cast<int>(len), 0), ec);
    if (nwrote >= 0)
      ec = std::error_code();
    return nwrote;
  }

  int read_from(socket_t sockfd, std::size_t len,
      void* buf, void* addr, std::error_code& ec, bool with_v6) {
    if (sockfd == kInvalidSocket) {
      ec = std::make_error_code(std::errc::bad_file_descriptor);
      return kSocketError;
    }

    socklen_t addrlen{};
    if (with_v6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    auto nread = error_wrapper(::recvfrom(sockfd,
          static_cast<char*>(buf), static_cast<int>(len), 0,
          static_cast<struct sockaddr*>(addr), &addrlen), ec);
    if (nread >= 0)
      ec = std::error_code();
    return nread;
  }

  int write_to(socket_t sockfd,
      const void* buf, std::size_t len, const void* addr, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = std::make_error_code(std::errc::bad_file_descriptor);
      return kSocketError;
    }

    auto* to_addr = static_cast<const struct sockaddr*>(addr);
    socklen_t addrlen{};
    if (to_addr->sa_family == AF_INET6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    clear_last_errno();
    auto nwrote = error_wrapper(::sendto(sockfd,
          static_cast<const char*>(buf),
          static_cast<int>(len), 0, to_addr, addrlen), ec);
    if (nwrote >= 0)
      ec = std::error_code();
    return nwrote;
  }

  bool set_non_blocking(socket_t sockfd, bool mode, std::error_code& ec) {
    if (sockfd == kInvalidSocket) {
      ec = std::make_error_code(std::errc::bad_file_descriptor);
      return false;
    }

    clear_last_errno();
    u_long flags = mode ? 1 : 0;
    int r = error_wrapper(::ioctlsocket(sockfd, FIONBIO, &flags), ec);

    if (r >= 0) {
      ec = std::error_code();
      return true;
    }
    return false;
  }
}

void clear_last_errno(void) {
  ::WSASetLastError(0);
}

int get_errno(void) {
  return ::WSAGetLastError();
}

const char* inet_ntop(int family, const void* addr, int len, char* buf) {
  return ::InetNtopA(family, addr, buf, static_cast<std::size_t>(len));
}

int inet_pton(int family, const char* buf, void* addr) {
  return ::InetPtonA(family, buf, addr);
}

int poll(PollFd* fds, int nfds, int timeout) {
  return ::WSAPoll(reinterpret_cast<WSAPOLLFD*>(fds),
      static_cast<ULONG>(nfds), timeout);
}

}
