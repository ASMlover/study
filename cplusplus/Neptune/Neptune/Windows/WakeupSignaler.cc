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
#include <Chaos/Logging/Logging.h>
#include <Neptune/Windows/WakeupSignaler.h>

namespace Neptune {

WakeupSignaler::WakeupSignaler(void) {
  open_signaler();
  if (!is_opened())
    CHAOSLOG_SYSFATAL << "WakeupSignaler::WakeupSignaler - open signaler failed";
}

WakeupSignaler::~WakeupSignaler(void) {
  close_signaler();
}

int WakeupSignaler::set_signal(const void* buf, std::size_t len) {
  return send(writfd_, (const char*)buf, len, 0);
}

int WakeupSignaler::get_signal(std::size_t len, void* buf) {
  return recv(readfd_, (char*)buf, len, 0);
}

void WakeupSignaler::open_signaler(void) {
  struct sockaddr_in addr{};
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_family = AF_INET;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    CHAOSLOG_SYSFATAL << "WakeupSignaler::open_signaler - create listening sockfd failed, sockfd=" << sockfd;
  int r = bind(sockfd, (const struct sockaddr*)&addr, sizeof(addr));
  if (r == 0) {
    int addrlen = sizeof(addr);
    r = getsockname(sockfd, (struct sockaddr*)&addr, &addrlen);
  }
  else {
    CHAOSLOG_SYSFATAL << "WakeupSignaler::open_signaler - bind sockfd failed, errno=" << WSAGetLastError();
  }
  if (r == 0)
    r = listen(sockfd, 1);

  writfd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (writfd_ < 0)
    CHAOSLOG_SYSFATAL << "WakeupSignaler::open_signaler - create writer sockfd failed, writfd_=" << writfd_;

  if (r == 0)
    r = connect(writfd_, (const struct sockaddr*)&addr, sizeof(addr));
  if (r == 0)
    readfd_ = accept(sockfd, nullptr, nullptr);
  if (readfd_ < 0)
    close_signaler();

   closesocket(sockfd);
}

void WakeupSignaler::close_signaler(void) {
  if (readfd_ > 0) {
    closesocket(readfd_);
    readfd_ = 0;
  }
  if (writfd_ > 0) {
    closesocket(writfd_);
    writfd_ = 0;
  }
}

}
