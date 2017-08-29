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
#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_) \
  || defined(_MSC_VER)
# define NETOP_WINDOWS
#else
# define NETOP_POSIX
#endif

#if defined(NETOP_WINDOWS)
# include <WinSock2.h>
# include <WS2tcpip.h>

# define inet_pton InetPtonA
# define inet_ntop InetNtopA
#else
# include <arpa/inet.h>
#endif
#include <iostream>

void show_inet(void) {
  {
    // inet_addr & inet_ntoa
    // inet_aton not supported in Windows
    auto addr = inet_addr("127.0.0.1");
    std::cout
      << "use(inet_addr) addr=" << addr << std::endl
      << "use(inet_ntoa) addr=" << inet_ntoa(*(struct in_addr*)&addr)
      << std::endl;
  }

  {
    // inet_pton & inet_ntop
    struct in_addr addr;
    inet_pton(AF_INET, "127.0.0.1", &addr);
    std::cout << "use(inet_pton) addr=" << addr.S_un.S_addr << std::endl;

    char buf[64]{};
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    std::cout << "use(inet_ntop) addr=" << buf << std::endl;
  }
}
