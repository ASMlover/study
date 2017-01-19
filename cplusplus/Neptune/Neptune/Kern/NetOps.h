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
#ifndef NEPTUNE_KERN_NETOPS_H
#define NEPTUNE_KERN_NETOPS_H

#include <Chaos/Platform.h>
#if defined(CHAOS_WINDOWS)
# include <WS2tcpip.h>
# if !defined(_WINDOWS_)
#   include <WinSock2.h>
# endif

  typedef int socklen_t;
  typedef int sa_family_t;
  typedef int in_addr_t;
  typedef SSIZE_T ssize_t;
#else
# include <netinet/tcp.h> // for TCP_NODELAY and etc
# include <netdb.h> // for sockaddr_in6 and etc
# include <poll.h>
#endif
#include <string>

namespace Neptune { namespace NetOps {

#if defined(CHAOS_WINDOWS)
  typedef WSABUF Iovec_t;
  typedef WSAPOLLFD Pollfd_t;
#else
  typedef struct iovec Iovec_t;
  typedef struct pollfd Pollfd_t;
#endif

namespace socket {
  // socket operations wrapper
  int open(sa_family_t family);
  int shutdown_read(int sockfd);
  int shutdown_write(int sockfd);
  int shutdown_all(int sockfd);
  int close(int sockfd);
  int bind(int sockfd, const struct sockaddr* addr);
  int listen(int sockfd);
  int accept(int sockfd, struct sockaddr_in6* addr);
  int connect(int sockfd, const struct sockaddr* addr);
  ssize_t read(int sockfd, std::size_t len, void* buf);
  ssize_t write(int sockfd, const void* buf, std::size_t len);
  void set_iovec(Iovec_t& vec, char* buf, std::size_t len);
  ssize_t readv(int sockfd, int niov, Iovec_t* iov);
  void set_nonblock(int sockfd);
  int set_option(int sockfd, int level, int optname, int optval);
  int get_option(int sockfd, int level, int optname, int* optval, socklen_t* optlen);
  int get_errno(int sockfd);
  struct sockaddr_in6 get_local(int sockfd);
  struct sockaddr_in6 get_peer(int sockfd);
  bool is_self_connect(int sockfd);
}

namespace addr {
  std::string to_string(const struct sockaddr* addr, bool is_only);
  std::string to_string(const struct sockaddr* addr);
  void get_address(const char* ip, std::uint16_t port, struct sockaddr_in* addr);
  void get_address(const char* ip, std::uint16_t port, struct sockaddr_in6* addr);
  struct sockaddr* cast(struct sockaddr_in* addr);
  const struct sockaddr* cast(const struct sockaddr_in* addr);
  struct sockaddr* cast(struct sockaddr_in6* addr);
  const struct sockaddr* cast(const struct sockaddr_in6* addr);
  const struct sockaddr_in* to_v4(const struct sockaddr* addr);
  const struct sockaddr_in6* to_v6(const struct sockaddr* addr);
}

int poll(Pollfd_t fds[], std::uint32_t nfds, int timeout);

}}

#endif // NEPTUNE_KERN_NETOPS_H
