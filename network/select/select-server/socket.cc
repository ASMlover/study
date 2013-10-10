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
#include "global.h"
#include "socket.h"



Socket::Socket(void)
  : fd_(INVALID_SOCKET)
{
}

Socket::~Socket(void)
{
}


void 
Socket::Bind(const char* ip, unsigned short port)
{
  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = inet_addr(ip);
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);

  if (SOCKET_ERROR == bind(fd_, 
        (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    LOG_ERR("bind failed err-code (%d)\n", WSAGetLastError());
  }
}

void 
Socket::Listen(void)
{
  if (SOCKET_ERROR == listen(fd_, SOMAXCONN)) {
    LOG_ERR("listen failed err-code (%d)\n", WSAGetLastError());
  }
}

SOCKET 
Socket::Accept(void)
{
  struct sockaddr_in remote_addr;
  int addrlen = sizeof(remote_addr);

  SOCKET s = accept(fd_, (struct sockaddr*)&remote_addr, &addrlen);
  if (INVALID_SOCKET == s) {
    LOG_ERR("accept failed err-code (%d)\n", WSAGetLastError());
  }

  return s;
}

bool 
Socket::Connect(const char* ip, unsigned short port)
{
  struct sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr(ip);
  remote_addr.sin_family      = AF_INET;
  remote_addr.sin_port        = htons(port);

  if (SOCKET_ERROR == connect(fd_, 
        (struct sockaddr*)&remote_addr, sizeof(remote_addr))) {
    LOG_ERR("connect failed err-code (%d)\n", WSAGetLastError());
    return false;
  }

  return true;
}
