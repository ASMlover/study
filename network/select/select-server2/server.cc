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



class Connector : public EventHandler {
public:
  virtual void ReadEvent(Socket* s)
  {
    char buf[128] = {0};
    if (SOCKET_ERROR == s->Read(sizeof(buf), buf))
      s->Close();
    fprintf(stdout, "recv from client : %s\n", buf);

    s->Write(buf, strlen(buf));
  }

  virtual void WriteEvent(Socket* s)
  {
  }
};


void 
ServerMain(const char* ip = NULL, unsigned short port = 5555)
{
  Connector c;
  Network network;

  network.Attach(&c);
  network.Init();
  network.Listen(ip, port);

  fprintf(stdout, "server<%s, %d> starting ...\n",
      (NULL == ip ? "INADDR_ANY" : ip), port);
  while (true)
    Sleep(100);

  network.Destroy();
}
