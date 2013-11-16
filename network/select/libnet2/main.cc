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
#include "os_tool.h"
#include "message.h"
#include "net.h"



class ConnectorHandler : public EventHandler {
public:
  virtual bool AcceptEvent(Socket* s, Address* addr)
  {
    fprintf(stdout, "accept connector [%d] from <%s, %d>\n",
        s->fd(), addr->ip(), addr->port());
    return true;
  }

  virtual void CloseEvent(Socket* s)
  {
    fprintf(stdout, "connector [%d] closed\n", s->fd());
  }

  virtual bool ReadEvent(Socket* s)
  {
    MessageHeader header;
    char buf[128] = {0};

    if (s->Read(sizeof(header), (char*)&header) > 0) {
      if (s->Read(header.size, buf) > 0) {
        fprintf(stdout, "recv from connector [%d] : %s\n", 
            s->fd(), buf);


        s->Write((const char*)&header, sizeof(header));
        s->Write(buf, strlen(buf));
      }
    }

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
  if (network.Listen(ip, port)) {
    fprintf(stdout, "server starting <%s, %d> ...\n", ip, port);
  }
  else {
    fprintf(stderr, "server starting failed ...\n");
    return;
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

  if (s.Connect(ip, port)) {
    fprintf(stdout, "connect to server <%s, %d> success ...\n", ip, port);
  }
  else {
    fprintf(stderr, "connect to server failed ...\n");
    return;
  }

  char buf[128];
  struct timeb tb;
  struct tm* now;
  MessageHeader header;
  while (true) {
    ftime(&tb);
    now = localtime(&tb.time);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
        now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, 
        now->tm_hour, now->tm_min, now->tm_sec, tb.millitm);
    header.size = strlen(buf);
    s.WriteBlock((const char*)&header, sizeof(header));
    s.WriteBlock(buf, header.size);

    memset(&header, 0, sizeof(header));
    memset(buf, 0, sizeof(buf));
    if (kNetTypeError == s.ReadBlock(sizeof(header), (char*)&header))
      break;
    if (kNetTypeError == s.ReadBlock(header.size, buf))
      break;
    fprintf(stdout, "recv from server : %s\n", buf);

    Tools::Sleep(0);
  }

  s.Close();
}



static bool _s_connected = false;
class ClientHandler : public EventHandler {
public:
  virtual void CloseEvent(Socket* s)
  {
    fprintf(stdout, "client [%d] closed\n", s->fd());
    _s_connected = false;
  }

  virtual bool ReadEvent(Socket* s)
  {
    MessageHeader header;
    char buf[128] = {0};

    if (s->Read(sizeof(header), (char*)&header) > 0) {
      if (s->Read(header.size, buf) > 0) {
        fprintf(stdout, "recv from server [%d] : %s\n", 
            s->fd(), buf);
      }
    }

    return true;
  }
};


static void 
AsyncClientMain(const char* ip = "127.0.0.1", unsigned short port = 5555)
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
    fprintf(stdout, "connect to server<%s, %d> success ...\n", ip, port);
  }
  _s_connected = true;


  char buf[128];
  struct timeb tb;
  struct tm* now;
  MessageHeader header;
  while (_s_connected) {
    ftime(&tb);
    now = localtime(&tb.time);
    sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
        now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, 
        now->tm_hour, now->tm_min, now->tm_sec, tb.millitm);
    header.size = strlen(buf);
    s->Write((const char*)&header, sizeof(header));
    s->Write(buf, header.size);

    Tools::Sleep(0);
  }
}


int 
main(int argc, char* argv[])
{
  if (argc < 2)  {
    fprintf(stderr, "usage : test [option ...]\n");
    return 0;
  }

  NetLibrary::Singleton().Init();

  if (0 == strcmp(argv[1], "srv"))
    ServerMain();
  else if (0 == strcmp(argv[1], "clt"))
    ClientMain();
  else if (0 == strcmp(argv[1], "async-clt"))
    AsyncClientMain();

  NetLibrary::Singleton().Destroy();
  return 0;
}
