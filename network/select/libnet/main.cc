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
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include "net.h"
#include "tools.h"


class ConnectorHandler : public EventHandler {
public:
  virtual bool AcceptEvent(Socket* s, Address* addr)
  {
    fprintf(stdout, "accept from<%s, %d> socket[%d]\n", 
        addr->ip(), addr->port(), s->fd());

    return true;
  }

  virtual void CloseEvent(Socket* s)
  {
    fprintf(stdout, "connector [%d] closed\n", s->fd());
  }

  virtual bool ReadEvent(Socket* s)
  {
    Packet packet;
    if (s->ReadPacket(&packet)) {
      fprintf(stdout, "recv from client : %s\n", packet.GetData());

      s->WritePacket(&packet);

      packet.ReleaseData();
    }

    return true;
  }
};


void 
ServerMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
{
  ConnectorHandler ch;
  SelectNetwork network;

  network.Attach(&ch);
  network.Init();
  
  if (network.Listen(ip, port)) {
    fprintf(stdout, "server <%s, %d> starting ...\n", ip, port);
  }
  else {
    fprintf(stderr, "server start failed ...\n");
    return;
  }

  while (true) 
    ToolsLib::Sleep(100);

  network.Destroy();
}





static bool _s_connected = false;
class ClientHandler : public EventHandler {
public:
  virtual bool ReadEvent(Socket* s)
  {
    Packet packet;
    if (s->ReadPacket(&packet)) {
      fprintf(stdout, "recv from server : %s\n", packet.GetData());

      packet.ReleaseData();
    }

    return true;
  }

  virtual void CloseEvent(Socket* s)
  {
    fprintf(stdout, "client [%d] closed\n", s->fd());
    _s_connected = false;
  }
};

void 
ClientMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
{
  ClientHandler ch;
  SelectNetwork network;

  network.Attach(&ch);
  network.Init();

  Socket* s = network.Connect(ip, port);
  if (NULL == s) {
    fprintf(stderr, "connect to server failed ...\n");
    return;
  }
  else {
    fprintf(stdout, "connect to server success ...\n");
  }
  _s_connected = true;

  char buf[128];
  struct timeb tb;
  struct tm* now;
  while (_s_connected) {
    ftime(&tb);
    now = localtime(&tb.time);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
        now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, 
        now->tm_hour, now->tm_min, now->tm_sec, tb.millitm);
    if (!s->WritePacket(buf, strlen(buf)))
      break;

    ToolsLib::Sleep(10);
  }
}


int 
main(int argc, char* argv[])
{
  NetLibrary::Singleton().Init();
  if (argc < 2)
    return 0;

  if (0 == strcmp("srv", argv[1]))
    ServerMain();
  else if (0 == strcmp("clt", argv[1]))
    ClientMain();

  NetLibrary::Singleton().Destroy();
  return 0;
}
