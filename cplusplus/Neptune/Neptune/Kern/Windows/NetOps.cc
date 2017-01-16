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
#include <WinSock2.h>
#include <Chaos/Logging/Logging.h>
#include <Neptune/Endian.h>
#include <Neptune/Kern/NetOps.h>

namespace Neptune { namespace NetOps {

namespace socket {
  int shutdown_read(int sockfd) {
    if (shutdown(sockfd, SD_RECEIVE) < 0)
      CHAOSLOG_SYSERR << "NetOps::socket::shutdown_read - errno=" << get_errno(sockfd);
    return 0;
  }

  int shutdown_write(int sockfd) {
    if (shutdown(sockfd, SD_SEND) < 0)
      CHAOSLOG_SYSERR << "NetOps::socket::shutdown_write - errno=" << get_errno(sockfd);
    return 0;
  }

  int shutdown_all(int sockfd) {
    if (shutdown(sockfd, SD_BOTH) < 0)
      CHAOSLOG_SYSERR << "NetOps::socket::shutdown_all - errno=" << get_errno(sockfd);
    return 0;
  }

  int close(int sockfd) {
    if (closesocket(sockfd) < 0)
      CHAOSLOG_SYSERR << "NetOps::socket::close - errno=" << get_errno(sockfd);
    return 0;
  }

  ssize_t read(int sockfd, std::size_t len, void* buf) {
    return recv(sockfd, static_cast<char*>(buf), static_cast<int>(len), 0);
  }

  ssize_t write(int sockfd, const void* buf, std::size_t len) {
    return send(sockfd, static_cast<const char*>(buf), static_cast<int>(len), 0);
  }

  void set_iovec(Iovec_t& vec, char* buf, std::size_t len) {
    vec.buf = buf;
    vec.len = len;
  }

  ssize_t readv(int sockfd, int niov, Iovec_t* iov) {
    DWORD read_bytes = 0;
    DWORD flags = 0;
    int rc = WSARecv(sockfd, iov, niov, &read_bytes, &flags, NULL, NULL);
    if (rc == SOCKET_ERROR)
      return rc;
    return static_cast<ssize_t>(read_bytes);
  }

  void set_nonblock(int sockfd) {
    u_long val{1};
    ioctlsocket(sockfd, FIONBIO, &val);
  }

  int set_option(int sockfd, int level, int optname, int optval) {
    return setsockopt(sockfd, level, optname, (const char*)&optval, sizeof(optval));
  }

  int get_option(int sockfd, int level, int optname, int* optval, socklen_t* optlen) {
    return getsockopt(sockfd, level, optname, (char*)optval, optlen);
  }
}

namespace addr {
  std::string to_string(const struct sockaddr* addr, bool /*ip_only*/) {
    char buf[64]{};
    if (addr->sa_family == AF_INET) {
      const struct sockaddr_in* addr4 = to_v4(addr);
      InetNtop(AF_INET, (PVOID)&addr4->sin_addr, buf, sizeof(buf));
    }
    else if (addr->sa_family == AF_INET6) {
      const struct sockaddr_in6* addr6 = to_v6(addr);
      InetNtop(AF_INET6, (PVOID)&addr6->sin6_addr, buf, sizeof(buf));
    }
    return buf;
  }

  void get_address(const char* ip, std::uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = Neptune::h2n16(port);
    if (InetPton(AF_INET, ip, &addr->sin_addr) <= 0)
      CHAOSLOG_SYSERR << "NetOps::addr::get_address(ipv4) - failed";
  }

  void get_address(const char* ip, std::uint16_t port, struct sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = Neptune::h2n16(port);
    if (InetPton(AF_INET6, ip, &addr->sin6_addr) <= 0)
      CHAOSLOG_SYSERR << "NetOps::addr::get_address(ipv6) - failed";
  }
}

int poll(Pollfd_t fds[], std::uint32_t nfds, int timeout) {
  return WSAPoll(fds, (ULONG)nfds, timeout);
}

}}
