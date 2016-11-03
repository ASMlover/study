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

#include <arpa/inet.h>

namespace tyr { namespace net {

namespace SocketSupport {
  int tyr_socket(sa_family_t family);
  int tyr_connect(int fd, const struct sockaddr* addr);
  void tyr_bind(int fd, const struct sockaddr* addr);
  void tyr_listen(int fd);
  int tyr_accept(int fd, struct sockaddr_in6* addr);
  ssize_t tyr_read(int fd, void* buf, size_t len);
  ssize_t tyr_readv(int fd, const struct iovec* iov, int iovcnt);
  ssize_t tyr_write(int fd, const void* buf, size_t len);
  void tyr_close(int fd);
  void tyr_shutdown(int fd);

  void tyr_to_ip_port(char* buf, size_t len, const struct sockaddr* addr);
  void tyr_to_ip(char* buf, size_t len, const struct sockaddr* addr);
  void tyr_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in* addr);
  void tyr_from_ip_port(const char* ip, uint16_t port, struct sockaddr_in6* addr);

  int tyr_socket_error(int fd);

  const struct sockaddr* tyr_sockaddr_cast(const struct sockaddr_in* addr);
  const struct sockaddr* tyr_sockaddr_cast(const struct sockaddr_in6* addr);
  struct sockaddr* tyr_sockaddr_cast(struct sockaddr_in6* addr);
  const struct sockaddr_in* tyr_sockaddr_in_cast(const struct sockaddr* addr);
  const struct sockaddr_in6* tyr_sockaddr_in6_cast(const struct sockaddr* addr);

  struct sockaddr_in6 tyr_localaddr(int fd);
  struct sockaddr_in6 tyr_peekaddr(int fd);
  bool tyr_is_self_connect(int fd);
}

}}

#endif // __TYR_NET_SOCKETSUPPORT_HEADER_H__
