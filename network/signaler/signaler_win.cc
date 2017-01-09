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
#include <Winsock2.h>
#include <iostream>

// build command:
// \> cl.exe /Fe"a.exe" signaler_win.cc /EHsc ws2_32.lib

#define UNUSED(x) ((void)x)

struct _EventFd_t {
  int writer{};
  int reader{};

  bool is_valid(void) const {
    return writer > 0 && reader > 0;
  }

  _EventFd_t& operator=(std::nullptr_t) {
    writer = 0;
    reader = 0;
  }
};

void eventfd_close(_EventFd_t eventfd) {
  if (eventfd.reader > 0)
    closesocket(eventfd.reader);
  if (eventfd.writer > 0)
    closesocket(eventfd.writer);
}

_EventFd_t eventfd_create(void) {
  struct sockaddr_in addr{};
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_family = AF_INET;

  bool has_error = false;
  _EventFd_t eventfd{};
  int listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0)
    return eventfd;

  int r = bind(listener, (const struct sockaddr*)&addr, sizeof(addr));
  if (0 == r) {
    int addrlen = sizeof(addr);
    r = getsockname(listener, (struct sockaddr*)&addr, &addrlen);
  }
  if (0 == r)
    r = listen(listener, 1);

  eventfd.writer = socket(AF_INET, SOCK_STREAM, 0);
  if (eventfd.writer < 0)
    goto Exit;
  if (0 == r)
    r = connect(eventfd.writer, (struct sockaddr*)&addr, sizeof(addr));
  if (0 == r)
    eventfd.reader = accept(listener, nullptr, nullptr);
  if (eventfd.reader <= 0)
    has_error = true;

Exit:
  if (has_error)
    eventfd_close(eventfd);
  if (listener > 0)
    closesocket(listener);

  return eventfd;
}

int eventfd_read(_EventFd_t eventfd, char* buf, size_t len) {
  return recv(eventfd.reader, buf, len, 0);
}

int eventfd_write(_EventFd_t eventfd, const char* buf, size_t len) {
  return send(eventfd.writer, buf, len, 0);
}

int main(int argc, char* argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  WSADATA wd;
  WSAStartup(MAKEWORD(2, 2), &wd);

  static const int RUN_TIMES = 100;
  _EventFd_t eventfd = eventfd_create();

  for (int i = 0; i < RUN_TIMES; ++i) {
    char wbuf[8] = "Wakeup!";
    eventfd_write(eventfd, wbuf, sizeof(wbuf));

    char rbuf[8] = {0};
    eventfd_read(eventfd, rbuf, sizeof(rbuf));
    std::cout << "[" << i+1 << "] read buffer is : " << rbuf << std::endl;
  }

  eventfd_close(eventfd);

  WSACleanup();
  return 0;
}
