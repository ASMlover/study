//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef _WINDOWS_
# include <winsock2.h>
#endif
#include <stdio.h>
#include <string.h>
#include "global.h"



void 
ClientMain(void)
{
  SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  remote_addr.sin_family      = AF_INET;
  remote_addr.sin_port        = htons(5555);
  if (SOCKET_ERROR == connect(fd, 
        (struct sockaddr*)&remote_addr, sizeof(remote_addr)))
    LOG_FAIL("connect failed err-code (%d)\n", WSAGetLastError());

  char buf[128];
  SYSTEMTIME s;
  while (true) {
    GetLocalTime(&s);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
        s.wYear, s.wMonth, s.wDay, s.wHour, 
        s.wMinute, s.wSecond, s.wMilliseconds);
    send(fd, buf, strlen(buf), 0);

    memset(buf, 0, sizeof(buf));
    if (SOCKET_ERROR == recv(fd, buf, sizeof(buf), 0))
      break;
    fprintf(stdout, "from server: %s\n", buf);

    Sleep(100);
  }

  closesocket(fd);
}
