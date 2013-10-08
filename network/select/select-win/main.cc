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
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#define LOG_ERR(fmt, ...) do {\
  va_list ap;\
  va_start(ap, (fmt));\
  char buf[1024];\
  vsprintf(buf, (fmt), ap);\
  va_end(ap);\
  fprintf(stderr, \
      "[%s] [%d] : %s", \
      __FILE__, \
      __LINE__, \
      buf);\
} while (0)


class SockInit {
  WSADATA wsaData_;

  SockInit(const SockInit&);
  SockInit& operator =(const SockInit&);
public:
  SockInit(void)
  {
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData_)) {
      LOG_ERR("WSAStartup failed ...\n");
      abort();
    }
  }

  ~SockInit(void)
  {
    WSACleanup();
  }
};


static SockInit _s_si;


static void 
ServerMain(const char* ip, unsigned short port)
{
  SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == listener) {
    LOG_ERR("socket failed ...\n");
    abort();
  }

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = inet_addr(ip);
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);
  if (SOCKET_ERROR == bind(listener, 
        (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    LOG_ERR("bind failed ...\n");
    abort();
  }
  if (SOCKET_ERROR == listen(listener, SOMAXCONN)) {
    LOG_ERR("listen failed ...\n");
    abort();
  }

  fd_set rset;  //!< read set
  fprintf(stdout, "server initialized ...\n");
  while (true) {
    FD_ZERO(&rset);
    FD_SET(listener, &rset);
    int ret = select(0, &rset, NULL, NULL, NULL);
    if (ret <= 0) {
      LOG_ERR("select failed ...\n");
      abort();
    }

    if (FD_ISSET(listener, &rset)) {
      struct sockaddr_in remote_addr;
      int addrlen = sizeof(remote_addr);
      SOCKET s = accept(listener, (struct sockaddr*)&remote_addr, &addrlen);
      if (INVALID_SOCKET == s) {
        LOG_ERR("accept failed ...\n");
        abort();
      }

      fprintf(stdout, "address => [%s] [%d]\n", 
          inet_ntoa(remote_addr.sin_addr), s);

      closesocket(s);
    }
  }

  closesocket(listener);
}


static void 
ClientMain(const char* ip, unsigned short port) 
{
  SOCKET connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == connector) {
    LOG_ERR("socket failed ...\n");
    abort();
  }
  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = inet_addr(ip);
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);
  if (SOCKET_ERROR == connect(connector, 
        (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    LOG_ERR("connect to server failed ...\n");
    abort();
  }

  closesocket(connector);
}




int 
main(int argc, char* argv[])
{
  if (argc < 2) {
    LOG_ERR("arguments failed ...\n");
    abort();
  }

  if (0 == strcmp(argv[1], "srv")) 
    ServerMain("127.0.0.1", 5555);
  else if (0 == strcmp(argv[1], "clt"))
    ClientMain("127.0.0.1", 5555);

  return 0;
}
