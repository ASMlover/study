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
#include "net.h"

class ConnectorHandler : public EventHandler {
public:
  virtual bool AcceptEvent(int fd, sockaddr* addr)
  {
    fprintf(stdout, "accept client from <%s> [%d]\n", addr->sa_data, fd);
    return true;
  }

  virtual void CloseEvent(int fd) 
  {
    fprintf(stdout, "client[%d] closeed\n", fd);
  }

  virtual bool ReadEvent(Socket* s)
  {
    char buf[128] = {0};
    if (kNetTypeError == s->Read(sizeof(buf), buf)) {
      s->Close();
      return false;
    }

    fprintf(stdout, "recv from client: %s\n", buf);

    s->Write(buf, strlen(buf));
    return true;
  }
};


static void 
ServerMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
{
  ConnectorHandler ch;
  SelectNetwork network;

  network.Attach(&ch);
  network.Init();
  network.Listen(ip, port);

  fprintf(stdout, "server <%s, %d> starting ...\n", ip, port);
  while (true) 
    Sleep(100);

  network.Destroy();
}

static void 
ClientMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
{
  Socket s;
  s.Open();

  if (s.Connect(ip, port)) {
    fprintf(stdout, "connect to server success ...\n");
  }
  else {
    fprintf(stderr, "connect to server failed ...\n");
    return;
  }

  SYSTEMTIME t;
  char buf[128];
  while (true) {
    GetLocalTime(&t);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
        t.wYear, t.wMonth, t.wDay, 
        t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    if (kNetTypeError == s.Write(buf, strlen(buf)))
      break;

    memset(buf, 0, sizeof(buf));
    if (kNetTypeError == s.Read(sizeof(buf), buf))
      break;
    fprintf(stdout, "recv from server: %s\n", buf);

    Sleep(100);
  }

  s.Close();
}

int 
main(int argc, char* argv[])
{
  if (argc < 2)
    return 0;

  NetLibrary::Singleton().Init();

  if (0 == strcmp("srv", argv[1]))
    ServerMain();
  else if (0 == strcmp("clt", argv[1]))
    ClientMain();

  NetLibrary::Singleton().Destroy();

  return 0;
}
