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

#define UNUSED(x) ((void)x)

int make_fdpair(int& w, int& r) {
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  int listener = socket(AF_INET, SOCK_STREAM, 0);
  bind(listener, (const struct sockaddr*)&addr, sizeof(addr));
  listen(listener, 1);
  int addrlen = sizeof(addr);
  getsockname(listener, (struct sockaddr*)&addr, &addrlen);

  w = socket(AF_INET, SOCK_STREAM, 0);
  connect(w, (struct sockaddr*)&addr, sizeof(addr));
  r = accept(listener, NULL, NULL);

  closesocket(listener);
  return 0;
}

int main(int argc, char* argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  WSADATA wd;
  WSAStartup(MAKEWORD(2, 2), &wd);

  static const int RUN_TIMES = 100;
  int w;
  int r;
  make_fdpair(w, r);

  for (int i = 0; i < RUN_TIMES; ++i) {
    char wbuf[8] = "Wakeup!";
    send(w, wbuf, sizeof(wbuf), 0);

    char rbuf[8] = {0};
    recv(r, rbuf, sizeof(rbuf), 0);
    std::cout << "[" << i+1 << "] read buffer is : " << rbuf << std::endl;
  }

  WSACleanup();
  return 0;
}
