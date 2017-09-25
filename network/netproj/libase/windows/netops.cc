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
#include <WinSock2.h>
#include <Chaos/IO/ColorIO.h>
#include "../netops.h"

namespace net {

void startup(void) {
  WSADATA wd;
  int r = WSAStartup(MAKEWORD(2, 2), &wd);

  if (r != 0) {
    namespace cc = Chaos::ColorIO;
    cc::fprintf(stderr, cc::ColorType::COLORTYPE_FG_RED,
        "net.startup - WinSock2 library startup failed\n");
  }
}

void cleanup(void) {
  WSACleanup();
}

namespace socket {
  int close(int sockfd) {
    return ::closesocket(sockfd);
  }

  ssize_t read(int sockfd, std::size_t len, void* buf) {
    return ::recv(sockfd, static_cast<char*>(buf), static_cast<int>(len), 0);
  }

  ssize_t write(int sockfd, const void* buf, std::size_t len) {
    return ::send(sockfd,
        static_cast<const char*>(buf), static_cast<int>(len), 0);
  }

  ssize_t readfrom(int sockfd,
      std::size_t len, void* buf, void* addr, bool v6) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (v6)
      addrlen = sizeof(struct sockaddr_in6);
    return ::recvfrom(sockfd,
        (char*)buf, (int)len, 0, (struct sockaddr*)addr, &addrlen);
  }

  ssize_t writeto(int sockfd,
      const void* buf, std::size_t len, const void* addr, bool v6) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (v6)
      addrlen = sizeof(struct sockaddr_in6);
    return ::sendto(sockfd,
        (const char*)buf, (int)len, 0, (const struct sockaddr*)addr, addrlen);
  }

  int set_option(int sockfd,
      int level, int optname, const void* optval, socklen_t optlen) {
    return ::setsockopt(sockfd,
        level, optname, (const char*)optval, optlen);
  }

  int get_option(int sockfd,
      int level, int optname, void* optval, socklen_t* optlen) {
    return ::getsockopt(sockfd,
        level, optname, (char*)optval, optlen);
  }

  void set_block(int sockfd) {
    u_long val{0};
    ::ioctlsocket(sockfd, FIONBIO, &val);
  }

  void set_nonblock(int sockfd) {
    u_long val{1};
    ::ioctlsocket(sockfd, FIONBIO, &val);
  }
}

namespace io {
  int poll(pollfd fds[], std::uint32_t nfds, int timeout) {
    return ::WSAPoll(fds, (ULONG)nfds, timeout);
  }
}

}
