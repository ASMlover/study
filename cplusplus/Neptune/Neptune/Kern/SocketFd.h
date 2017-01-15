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
#ifndef NEPTUNE_KERN_SOCKETFD_H
#define NEPTUNE_KERN_SOCKETFD_H

#include <Chaos/Platform.h>
#include <Chaos/Copyable.h>
#if defined(CHAOS_WINDOWS)
# include <WS2tcpip.h>
# include <WinSock2.h>

  typedef int socklen_t;
  typedef int sa_family_t;
  typedef int in_addr_t;
  typedef SSIZE_T ssize_t;
  typedef WSABUF Iovec_t;
#else
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/types.h>
# include <netdb.h>

  typedef struct iovec Iovec_t;
#endif
#include <string>

namespace Neptune {

class SocketFd : public Chaos::Copyable {
  int sockfd_{};

  void set_nonblock(int sockfd);
public:
  SocketFd(int sockfd)
    : sockfd_(sockfd) {
  }

  int get_sockfd(void) const {
    return sockfd_;
  }

  bool is_opened(void) const {
    return sockfd_ > 0;
  }

  void open(sa_family_t family);
  void shutdown_read(void);
  void shutdown_write(void);
  void shutdown_all(void);
  void close(void);
  void bind(const struct sockaddr* addr);
  void listen(void);
  int accept(struct sockaddr_in6* addr);
  int connect(const struct sockaddr* addr);
  ssize_t read(std::size_t len, void* buf);
  ssize_t write(const void* buf, std::size_t len);
  void set_iovec(Iovec_t& vec, char* buf, size_t len);
  ssize_t readv(int niov, Iovec_t* iov);

  void set_nonblock(void);
  bool set_option(int level, int optname, int optval);
  bool get_option(int level, int optname, int* optval, socklen_t* optlen);

  int get_errno(void);
  struct sockaddr_in6 get_local(void) const;
  struct sockaddr_in6 get_peer(void) const;
  bool is_self_connect(void) const;
};

}

#endif // NEPTUNE_KERN_SOCKETFD_H
