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
#pragma once

struct sockaddr;

namespace netpp {

void startup(void);
void cleanup(void);

namespace socket {
  static constexpr int SHUT_READ = 0;
  static constexpr int SHUT_WRITE = 1;
  static constexpr int SHUT_BOTH = 2;

  int open(int family, int socket_type, int protocol);
  int close(int sockfd);
  int shutdown(int sockfd, int how);
  int bind(int sockfd, const void* addr);
  int listen(int sockfd);
  int accept(int sockfd, void* addr, bool with_v6 = false);
  int connect(int sockfd, const void* addr);
  int read(int sockfd, int len, void* buf);
  int write(int sockfd, const void* buf, int len);
  int readfrom(int sockfd,
      int len, void* buf, void* addr, bool with_v6 = false);
  int writeto(int sockfd, const void* buf, int len, const void* addr);
}

const char* inet_ntop(int family, const void* addr, int len, char* buf);
int inet_pton(int family, const char* buf, void* addr);

struct PollFd {
  int fd{};
  short events{};
  short revents{};

  PollFd(int f, short ev, short rev)
    : fd(f)
    , events(ev)
    , revents(rev) {
  }
};
int poll(PollFd* fds, int nfds, int timeout);

}
