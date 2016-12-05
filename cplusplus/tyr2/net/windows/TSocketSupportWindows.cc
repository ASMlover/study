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
#include <WS2tcpip.h>
#include <WinSock2.h>
#include "../../basic/TLogging.h"
#include "../TEndian.h"
#include "../unexposed/TSocketSupportUnexposed.h"
#include "../TSocketSupport.h"

namespace tyr { namespace net {

namespace SocketSupport {
  namespace unexposed {
    void kern_set_nonblock(int sockfd) {
      u_long val = 1;
      ioctlsocket(sockfd, FIONBIO, &val);
    }
  }

  void kern_set_iovec(KernIovec* vec, char* buf, size_t len) {
    vec->buf = buf;
    vec->len = len;
  }

  ssize_t kern_read(int sockfd, void* buf, size_t len) {
    return recv(sockfd, static_cast<char*>(buf), static_cast<int>(len), 0);
  }

  ssize_t kern_readv(int sockfd, const KernIovec* iov, int iovcnt) {
    DWORD read_bytes = 0;
    DWORD flags = 0;
    int rc = WSARecv(sockfd, const_cast<LPWSABUF>(iov), iovcnt, &read_bytes, &flags, nullptr, nullptr);
    if (SOCKET_ERROR == rc)
      return rc;
    return static_cast<ssize_t>(read_bytes);
  }

  ssize_t kern_write(int sockfd, const void* buf, size_t len) {
    return send(sockfd, static_cast<const char*>(buf), static_cast<int>(len), 0);
  }

  int kern_shutdown(int sockfd) {
    int rc = shutdown(sockfd, SD_SEND);
    if (rc < 0)
      TL_SYSERR << "SocketSupport::kern_shutdown failed";
    return 0;
  }

  int kern_close(int sockfd) {
    int rc = closesocket(sockfd);
    if (rc < 0)
      TL_SYSERR << "SocketSupport::kern_close failed";
    return 0;
  }

  void kern_to_ip(char* buf, size_t len, const struct sockaddr* addr) {
    if (addr->sa_family == AF_INET) {
      const struct sockaddr_in* addr4 = kern_sockaddr_in_cast(addr);
      InetNtop(AF_INET, (PVOID)&addr4->sin_addr, buf, len);
    }
    else if (addr->sa_family == AF_INET6) {
      const struct sockaddr_in6* addr6 = kern_sockaddr_in6_cast(addr);
      InetNtop(AF_INET6, (PVOID)&addr6->sin6_addr, buf, len);
    }
  }

  void kern_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = host_to_net16(port);
    int rc = InetPton(AF_INET, ip, &addr->sin_addr);
    if (rc <= 0)
      TL_SYSERR << "SocketSupport::kern_from_ip_port(ipv4) failed";
  }

  void kern_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = host_to_net16(port);
    int rc = InetPton(AF_INET6, ip, &addr->sin6_addr);
    if (rc <= 0)
      TL_SYSERR << "SocketSupport::kern_from_ip_port(ipv6) failed";
  }

  int kern_poll(KernPollfd fds[], uint32_t nfds, int timeout) {
    return WSAPoll(fds, (ULONG)nfds, timeout);
  }
}

}}
