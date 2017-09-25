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
#include "netops.h"

namespace net {

namespace socket {
  int open(sa_family_t family, int socket_type, int protocal) {
    return static_cast<int>(::socket(family, socket_type, protocal));
  }

  int shutdown(int sockfd, int how) {
    return ::shutdown(sockfd, how);
  }

  int bind(int sockfd, const void* addr, bool v6) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (v6)
      addrlen = sizeof(struct sockaddr_in6);
    return ::bind(sockfd, (const struct sockaddr*)addr, addrlen);
  }

  int listen(int sockfd) {
    return ::listen(sockfd, SOMAXCONN);
  }

  int accept(int sockfd, void* addr, bool v6) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (v6)
      addrlen = sizeof(struct sockaddr_in6);
    return static_cast<int>(::accept(sockfd, (struct sockaddr*)addr, &addrlen));
  }

  int connect(int sockfd, const void* addr, bool v6) {
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (v6)
      addrlen = sizeof(struct sockaddr_in6);
    return ::connect(sockfd, (const struct sockaddr*)addr, addrlen);
  }
}

namespace io {
  int select(int nfds, fd_set* readfds, fd_set* writefds,
      fd_set* exceptfds, const struct timeval* timeout) {
    return ::select(nfds,
        readfds, writefds, exceptfds, (struct timeval*)timeout);
  }
}

}
