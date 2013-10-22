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
#include <windows.h>
#include <stdio.h>
#include "net.h"


void 
ClientMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
{
  Socket s;
  s.Open();
  
  if (s.Connect(ip, port)) {
    fprintf(stdout, "connect to server success ...\n");
  }
  else {
    fprintf(stderr, "connect to server failed ...\n");
    goto Exit;
  }

  char buf[128];
  SYSTEMTIME t;
  while (true) {
    GetLocalTime(&t);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
        t.wYear, t.wMonth, t.wDay, 
        t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    s.Write(buf, strlen(buf));

    memset(buf, 0, sizeof(buf));
    if (SOCKET_ERROR == s.Read(sizeof(buf), buf))
      break;
    fprintf(stdout, "recv from server : %s\n", buf);
  }

Exit:
  s.Close();
}
