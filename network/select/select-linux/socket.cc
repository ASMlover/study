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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "socket.h"




Socket::Socket(void)
  : fd_(0)
{
}

Socket::~Socket(void)
{
  Close();
}

bool 
Socket::Create(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  return (-1 != fd_);
}

void 
Socket::Close(void)
{
  shutdown(fd_, SHUT_RDWR);
  close(fd_);
}

bool 
Socket::Listen(const char* ip, unsigned int port)
{
  struct sockaddr_in addr;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  if (0 != bind(fd_, (struct sockaddr*)&addr, sizeof(addr)))
    return false;
  if (0 != listen(fd_, SOMAXCONN))
    return false;

  return true;
}

bool 
Socket::Connect(const char* ip, unsigned int port)
{
  struct sockaddr_in addr;

  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
  if (0 != connect(fd_, (struct sockaddr*)&addr, sizeof(addr)))
    return false;
  return true;
}

int 
Socket::Accept(void)
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  return accept(fd_, (struct sockaddr*)&addr, &len);
}

int 
Socket::Read(int len, char* buf)
{
  return recv(fd_, buf, len, 0);
}

int 
Socket::Write(const char* buf, int len)
{
  return send(fd_, buf, len, 0);
}
