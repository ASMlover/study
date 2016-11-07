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
#ifndef __TYR_NET_SOCKETSUPPORT_HEADER_H__
#define __TYR_NET_SOCKETSUPPORT_HEADER_H__

#include "../basic/TConfig.h"
#include "../basic/TTypes.h"
#if defined(TYR_WINDOWS)
# include <WS2tcpip.h>
# include <WinSock2.h>

  typedef SSIZE_T ssize_t;
  typedef WSABUF  KernIovec;
  typedef int     socklen_t;
#else
# include <sys/socket.h>
# include <sys/uio.h>

  typedef struct iovec KernIovec;
#endif

namespace tyr { namespace net {

namespace SocketSupport {
  void kern_set_iovec(KernIovec* vec, char* buf, size_t len);

  int kern_socket(int family);
  int kern_bind(int sockfd, const struct sockaddr* addr);
  int kern_listen(int sockfd);
  int kern_accept(int sockfd, struct sockaddr_in6* addr);
  int kern_connect(int sockfd, const struct sockaddr* addr);
  ssize_t kern_read(int sockfd, void* buf, size_t len);
  ssize_t kern_readv(int sockfd, const KernIovec* iov, int iovcnt);
  ssize_t kern_write(int sockfd, const void* buf, size_t len);
  int kern_shutdown(int sockfd);
  int kern_close(int sockfd);

  void kern_to_ip_port(char* buf, size_t len, const struct sockaddr* addr);
  void kern_to_ip(char* buf, size_t len, const struct sockaddr* addr);
  void kern_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in* addr);
  void kern_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in6* addr);

  int kern_socket_error(int sockfd);

  const struct sockaddr* kern_sockaddr_cast(const struct sockaddr_in* addr);
  const struct sockaddr* kern_sockaddr_cast(const struct sockaddr_in6* addr);
  struct sockaddr* kern_sockaddr_cast(struct sockaddr_in6* addr);
  const struct sockaddr_in* kern_sockaddr_in_cast(const struct sockaddr* addr);
  const struct sockaddr_in6* kern_sockaddr_in6_cast(const struct sockaddr* addr);

  struct sockaddr_in6 kern_localaddr(int sockfd);
  struct sockaddr_in6 kern_peekaddr(int sockfd);
  bool kern_is_self_connect(int sockfd);
}

}}

#endif // __TYR_NET_SOCKETSUPPORT_HEADER_H__
