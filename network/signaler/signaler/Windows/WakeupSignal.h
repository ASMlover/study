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
#ifndef WINDOWS_WAKEUPSIGNAL_H_
#define WINDOWS_WAKEUPSIGNAL_H_

#include <Winsock2.h>

class WakeupSignal {
  int rfd_{}; // reader
  int wfd_{}; // writer

  void open(void) {
    struct sockaddr_in addr{};
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_family = AF_INET;

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
      return;

    int r = bind(listener, (const struct sockaddr*)&addr, sizeof(addr));
    if (0 == r) {
      int addrlen = sizeof(addr);
      r = getsockname(listener, (struct sockaddr*)&addr, &addrlen);
    }
    if (0 == r)
      r = listen(listener, 1);

    wfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (wfd_ < 0) {
      close();
      return;
    }
    if (0 == r)
      r = connect(wfd_, (struct sockaddr*)&addr, sizeof(addr));
    if (0 == r)
      rfd_ = accept(listener, nullptr, nullptr);
    if (rfd_ < 0)
      close();

    closesocket(listener);
  }

  void close(void) {
    if (rfd_ > 0) {
      closesocket(rfd_);
      rfd_ = 0;
    }
    if (wfd_ > 0) {
      closesocket(wfd_);
      wfd_ = 0;
    }
  }
public:
  WakeupSignal(void) {
    open();
  }

  ~WakeupSignal(void) {
    close();
  }

  int get_reader_fd(void) const {
    return rfd_;
  }

  bool is_opened(void) const {
    return rfd_ > 0 && wfd_ > 0;
  }

  int set_signal(const char* buf, size_t len) {
    return send(wfd_, buf, len, 0);
  }

  int get_signal(size_t len, char* buf) {
    return recv(rfd_, buf, len, 0);
  }
};

#endif // WINDOWS_WAKEUPSIGNAL_H_
