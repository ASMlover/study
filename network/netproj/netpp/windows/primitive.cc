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
  int close(int sockfd) {
    return ::closesocket(sockfd);
  }

  int read(int sockfd, int len, void* buf) {
    return ::recv(sockfd, static_cast<char*>(buf), len, 0);
  }

  int write(int sockfd, const void* buf, int len) {
    return ::send(sockfd, static_cast<const char*>(buf), len, 0);
  }

  int readfrom(int sockfd, int len, void* buf, void* addr, bool with_v6) {
    socklen_t addrlen{};
    if (with_v6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    return ::recvfrom(sockfd, static_cast<char*>(buf),
        len, 0, static_cast<struct sockaddr*>(addr), &addrlen);
  }

  int writeto(int sockfd, const void* buf, int len, const void* addr) {
    auto* to_addr = static_cast<const struct sockaddr*>(addr);

    socklen_t addrlen{};
    if (to_addr->sa_family == AF_INET6)
      addrlen = sizeof(struct sockaddr_in6);
    else
      addrlen = sizeof(struct sockaddr_in);

    return ::sendto(sockfd,
        static_cast<const char*>(buf), len, 0, to_addr, addrlen);
  }

  bool set_non_blocking(int sockfd, bool mode, std::error_code& ec) {
    if (sockfd == -1) {
      ec = std::make_error_code(std::errc::bad_file_descriptor);
      return false;
    }

    clear_last_errno();
    int flags = mode ? 1 : 0;
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
