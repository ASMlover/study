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
#include <sys/timeb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "net.h"
#include "os_tool.h"
#include "address.h"
#include "Socket.h"
#include "network.h"


class ConnectorHandler : public EventHandler {
public:
  virtual bool AcceptEvent(Socket* s, Address* addr)
  {
    fprintf(stdout, "accept connector [%d]\n", s->fd());
    return true;
  }

  virtual void CloseEvent(Socket* s) 
  {
    fprintf(stdout, "connector [%d] closed\n", s->fd());
  }

  virtual bool ReadEvent(Socket* s)
  {
    char buf[128] = {0};
    if (s->Read(sizeof(buf), buf) > 0) {
      fprintf(stdout, "recv from connector [%d] : %s\n", s->fd(), buf);

      s->Write(buf, strlen(buf));
    }

    return true;
  }
};

static void 
ServerMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
{
  ConnectorHandler ch;
  Network network;

  network.Attach(&ch);
  network.Init();

  if (!network.Listen(ip, port)) {
    fprintf(stdout, "server <%s, %d> starting failed ...\n", ip, port);
    return;
  }
  else {
    fprintf(stdout, "server <%s, %d> starting success ...\n", ip, port);
  }

  while (true) 
    Tools::Sleep(100);

  network.Destroy();
}

static void 
ClientMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
{
  Socket s;

  s.Open();

  if (!s.Connect(ip, port)) {
    fprintf(stdout, "connect to server<%s, %d> failed\n", ip, port);
    return;
  }
  else {
    fprintf(stdout, "connect to server<%s, %d> success\n", ip, port);
  }

  char buf[128];
  struct timeb tb;
  struct tm* now;
  while (true) {
    ftime(&tb);
    now = localtime(&tb.time);

    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
        now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, 
        now->tm_hour, now->tm_min, now->tm_sec, tb.millitm);
    s.WriteBlock(buf, strlen(buf));

    memset(buf, 0, sizeof(buf));
    if (kNetTypeError == s.ReadBlock(sizeof(buf), buf)) 
      break;
    fprintf(stdout, "recv form server : %s\n", buf);

    Tools::Sleep(10);
  }
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
