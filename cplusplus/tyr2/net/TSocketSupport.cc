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
#include "../basic/TConfig.h"
#if defined(TYR_WINDOWS)
# include <WinSock2.h>
# include <Windows.h>
#else
# include <sys/socket.h>
# include <fcntl.h>
# include <unistd.h>
#endif
#include <errno.h>
#include <stdio.h>
#include "../basic/TLogging.h"
#include "TEndian.h"
#include "unexposed/TSocketSupportUnexposed.h"
#include "TSocketSupport.h"

namespace tyr { namespace net {

namespace SocketSupport {
  int kern_socket(int family) {
    int sockfd = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
      TL_SYSFATAL << "SocketSupport::kern_socket failed";
    unexposed::kern_set_nonblock(sockfd);

    return sockfd;
  }

  int kern_bind(int sockfd, const struct sockaddr* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
    int rc = bind(sockfd, addr, addrlen);
    if (rc < 0)
      TL_SYSFATAL << "SocketSupport::kern_bind failed";
    return 0;
  }

  int kern_listen(int sockfd) {
    int rc = listen(sockfd, SOMAXCONN);
    if (rc < 0)
      TL_SYSFATAL << "SocketSupport::kern_listen failed";
    return 0;
  }

  int kern_accept(int sockfd, struct sockaddr_in6* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = accept(sockfd, kern_sockaddr_cast(addr), &addrlen);
    unexposed::kern_set_nonblock(connfd);

    if (connfd < 0) {
      int saved_errno = errno;
      TL_SYSERR << "SocketSupport::kern_accept failed";
      switch (saved_errno) {
      }
    }
    return connfd;
  }

  int kern_connect(int sockfd, const struct sockaddr* addr) {
    return connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  }

  void kern_to_ip_port(char* buf, size_t len, const struct sockaddr* addr) {
    kern_to_ip(buf, len, addr);
    size_t end = strlen(buf);
    const struct sockaddr_in* addr4 = kern_sockaddr_in_cast(addr);
    uint16_t port = net_to_host16(addr4->sin_port);
    snprintf(buf + end, len - end, ":%u", port);
  }

  int kern_socket_error(int sockfd) {
    char optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
      return errno;
    else
      return optval;
  }

  const struct sockaddr* kern_sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr*>(tyr::basic::implicit_cast<const void*>(addr));
  }

  const struct sockaddr* kern_sockaddr_cast(const struct sockaddr_in6* addr) {
    return static_cast<const struct sockaddr*>(tyr::basic::implicit_cast<const void*>(addr));
  }

  struct sockaddr* kern_sockaddr_cast(struct sockaddr_in6* addr) {
    return static_cast<struct sockaddr*>(tyr::basic::implicit_cast<void*>(addr));
  }

  const struct sockaddr_in* kern_sockaddr_in_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(tyr::basic::implicit_cast<const void*>(addr));
  }

  const struct sockaddr_in6* kern_sockaddr_in6_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in6*>(tyr::basic::implicit_cast<const void*>(addr));
  }

  struct sockaddr_in6 kern_localaddr(int sockfd) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    if (getsockname(sockfd, kern_sockaddr_cast(&addr), &addrlen) < 0)
      TL_SYSERR << "SocketSupport::kern_localaddr failed";
    return addr;
  }

  struct sockaddr_in6 kern_peekaddr(int sockfd) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    if (getpeername(sockfd, kern_sockaddr_cast(&addr), &addrlen) < 0)
      TL_SYSERR << "SocketSupport::kern_peekaddr failed";
    return addr;
  }

  bool kern_is_self_connect(int sockfd) {
    struct sockaddr_in6 localaddr = kern_localaddr(sockfd);
    struct sockaddr_in6 peeraddr = kern_peekaddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
      const struct sockaddr_in* laddr4 = reinterpret_cast<const struct sockaddr_in*>(&localaddr);
      const struct sockaddr_in* paddr4 = reinterpret_cast<const struct sockaddr_in*>(&peeraddr);
      return (laddr4->sin_port == paddr4->sin_port &&
          laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr);
    }
    else if (localaddr.sin6_family == AF_INET6) {
      return (localaddr.sin6_port == peeraddr.sin6_port &&
          0 == memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localaddr.sin6_addr)));
    }
    else {
      return false;
    }
  }
}

}}
