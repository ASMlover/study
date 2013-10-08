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
#include <vector>
#include "global.h"


class Server {
  SOCKET fd_;
  bool   running_;
  HANDLE thread_;
  HANDLE start_event_;
  fd_set rset_;
  std::vector<SOCKET> fds_;

  Server(const Server&);
  Server& operator =(const Server&);
public:
  explicit Server(void);
  ~Server(void);

  void Start(const char* ip, unsigned short port);
  void Stop(void);
  void Run(void);
private:
  static DWORD WINAPI Worker(void* arg);
};



Server::Server(void)
  : fd_(INVALID_SOCKET)
  , running_(false)
  , thread_(NULL)
  , start_event_(NULL)
{
  FD_ZERO(&rset_);
}

Server::~Server(void)
{
  Stop();
}

void 
Server::Start(const char* ip, unsigned short port)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == fd_) {
    LOG_ERR("socket failed ...\n");
    abort();
  }

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = inet_addr(ip);
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);
  if (SOCKET_ERROR == bind(fd_, 
      (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    LOG_ERR("bind failed ...\n");
    abort();
  }

  if (SOCKET_ERROR == listen(fd_, SOMAXCONN)) {
    LOG_ERR("listen failed ...\n");
    abort();
  }
}

void 
Server::Stop(void)
{
}

void 
Server::Run(void)
{
}



DWORD WINAPI 
Server::Worker(void* arg)
{
  return 0;
}





void 
ServerMain(const char* ip, unsigned short port)
{
  Server server;
  server.Start(ip, port);

  server.Run();

  server.Stop();
}
