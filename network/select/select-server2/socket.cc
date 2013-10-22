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
#include "common.h"
#include "socket.h"



Socket::Socket(void)
  : fd_(INVALID_SOCKET)
{
}

Socket::~Socket(void)
{
  Close();
}

int 
Socket::fd(void) const 
{
  return fd_;
}

void 
Socket::Attach(int fd)
{
  fd_ = fd;
}

int 
Socket::Detach(void)
{
  int fd = fd_;
  fd_ = INVALID_SOCKET;

  return fd;
}

void 
Socket::Open(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == fd_)
    LOG_FAIL("socket failed err-code(%d)\n", NetErrno());
}

void 
Socket::Close(void)
{
  if (INVALID_SOCKET != fd_) {
    shutdown(fd_, SD_BOTH);
    closesocket(fd_);

    fd_ = INVALID_SOCKET;
  }
}

void 
Socket::Bind(const char* ip, unsigned short port)
{
  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = 
    (NULL == ip ? htonl(INADDR_ANY) : inet_addr(ip));
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);

  if (SOCKET_ERROR == bind(fd_, 
        (struct sockaddr*)&host_addr, sizeof(host_addr)))
    LOG_FAIL("bind failed err-code(%d)\n", NetErrno());
}

void 
Socket::Listen(void)
{
  if (SOCKET_ERROR == listen(fd_, SOMAXCONN))
    LOG_FAIL("listen failed err-code(%d)\n", NetErrno());
}

bool 
Socket::Accept(Socket* s, struct sockaddr* addr)
{
  struct sockaddr_in remote_addr;
  int addrlen = sizeof(remote_addr);

  int tmp = accept(fd_, 
      (NULL != addr ? addr : (struct sockaddr*)&remote_addr), &addrlen);
  if (INVALID_SOCKET == tmp) {
    //! LOG_ERR("accept error err-code(%d)\n", NetErrno());
    return false;
  }

  s->Attach(tmp);
  return true;
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

  if (SOCKET_ERROR == connect(fd_, 
        (struct sockaddr*)&remote_addr, sizeof(remote_addr))) {
    LOG_FAIL("connect failed err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

int 
Socket::Read(int length, char* buffer)
{
  int ret = recv(fd_, buffer, length, 0);
  if (SOCKET_ERROR == ret)
    LOG_ERR("recv error err-code(%d)\n", NetErrno());

  return ret;
}

int 
Socket::Write(const char* buffer, int length)
{
  int total = 0;
  int ret;
  while (total < length) {
    ret = send(fd_, buffer + total, length - total, 0);
    if (SOCKET_ERROR == ret) {
      LOG_ERR("send error err-code(%d)\n", NetErrno());
      return SOCKET_ERROR;
    }

    total += ret;
  }

  return total;
}
