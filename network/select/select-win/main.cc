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

class SockInit {
  WSADATA wsaData_;

  SockInit(const SockInit&);
  SockInit& operator =(const SockInit&);
public:
  SockInit(void)
  {
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData_))
      abort();
  }

  ~SockInit(void)
  {
    WSACleanup();
  }
};




int 
main(int argc, char* argv[])
{
  SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in addr;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(5555);
  bind(listener, (struct sockaddr*)&addr, sizeof(addr));
  listen(listener, SOMAXCONN);

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(listener, &fds);

  while (true) {
    fd_set fdread = fds;
    int ret = select(0, &fdread, NULL, NULL, NULL);
    if (ret <= 0) {
      fprintf(stderr, "select failed ...\n");
      exit(-1);
    }

    for (int i = 0; i < (int)fds.fd_count; ++i) {
      if (FD_ISSET(fds.fd_array[i], &fdread)) {
        if (fds.fd_array[i] == listener) {
          if (fds.fd_count >= FD_SETSIZE) {
            fprintf(stderr, "too much connections ...\n");
            continue;
          }

          struct sockaddr_in addr_remote;
          int addrlen = sizeof(addr_remote);
          SOCKET s = accept(listener, 
              (struct sockaddr*)&addr_remote, &addrlen);
          FD_SET(s, &fds);

          fprintf(stdout, "new connection ... s = %d\n", s);
        }
        else {
          char buf[256] = {0};
          recv(fds.fd_array[i], buf, sizeof(buf), 0);

          fprintf(stdout, "recv message is : %s\n", buf);

          shutdown(fds.fd_array[i], SD_BOTH);
          closesocket(fds.fd_array[i]);
          FD_CLR(fds.fd_array[i], &fds);
        }
      }
    }
  }

  closesocket(listener);

  return 0;
}
