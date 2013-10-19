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
#include "common.h"
#include "socket.h"




Socket::Socket(void)
  : fd_(-1)
{
}

Socket::~Socket(void)
{
  Close();
}

void 
Socket::Open(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == fd_)
    LOG_FAIL("socket failed err-code(%d)\n", Errno());
}

void 
Socket::Close(void)
{
  shutdown(fd_, SHUT_RDWR);
  close(fd_);
  fd_ = -1;
}

void 
Socket::Bind(const char* ip, unsigned short port)
{
  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = 
    (NULL == ip ? htonl(INADDR_ANY) : inet_addr(ip));
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);

  if (0 != bind(fd_, (struct sockaddr*)&host_addr, sizeof(host_addr)))
    LOG_FAIL("bind failed err-code(%d)\n", Errno());
}

void 
Socket::Listen(void)
{
  if (0 != listen(fd_, SOMAXCONN))
    LOG_FAIL("listen failed err-code(%d)\n", Errno());
}

void 
Socket::Accept(Socket* s, struct sockaddr* addr)
{
  struct sockaddr_in remote_addr;
  socklen_t addrlen = sizeof(remote_addr);

  int tmp = accept(fd_, 
      (NULL != addr ? addr : (struct sockaddr*)&remote_addr), &addrlen);
  if (-1 == tmp)
    LOG_ERR("accept error err-code(%d)\n", Errno());

  s->Attach(tmp);
}

bool 
Socket::Connect(const char* ip, unsigned short port)
{
  if (NULL == ip)
    ip = "127.0.0.1";

  struct sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr(ip);
  remote_addr.sin_family      = AF_INET;
  remote_addr.sin_port        = htons(port);
  if (0 != connect(fd_, 
        (struct sockaddr*)&remote_addr, sizeof(remote_addr))) {
    LOG_ERR("connect error err-code(%d)\n", Errno());
    return false;
  }

  return true;
}

int 
Socket::Read(int len, char* buf)
{
  int ret = recv(fd_, buf, len, 0);
  if (ret < 0)
    LOG_ERR("recv error err-code(%d)\n", Errno());

  return ret;
}

int 
Socket::Write(const char* buf, int len)
{
  int total = 0;
  int ret = 0;
  while (total < len) {
    ret = send(fd_, buf + total, len - total, 0);
    if (ret < 0) {
      LOG_ERR("send error err-code(%d)\n", Errno());
      break;
    }

    total += ret;
  }

  return total;
}
