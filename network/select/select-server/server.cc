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
#include "socket.h"
#include "event_handler.h"
#include "network.h"



class ConnHandler : public EventHandler {
public:
  explicit ConnHandler(Socket* s)
    : EventHandler(s)
  {
  }

  virtual void ReadEvent(void)
  {
    char buf[128] = {0};
    s_->Read(128, buf);
    fprintf(stdout, "recv from client : %s\n", buf);

    int len = strlen(buf);
    s_->Write(buf, len);
  }

  virtual void WriteEvent(void)
  {
  }
};

static EventHandler* 
CreateConnHandler(Socket* s)
{
  s->SetBlocking(true);
  return new ConnHandler(s);
}



void 
ServerMain(void)
{
  Network net;
  net.Init(CreateConnHandler);

  net.Start("127.0.0.1", 5555);
  while (true)
    Sleep(100);
}
