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
#include "net.h"
#include "logging.h"
#include "socket.h"



Socket::Socket(void)
  : fd_(kNetTypeInvalid)
{
}

Socket::~Socket(void)
{
  Close();
}

bool 
Socket::SetTcpNoDelay(bool nodelay)
{
  if (kNetTypeInvalid == fd_)
    return false;

  int optval = (nodelay ? 1 : 0);
  if (kNetTypeError == setsockopt(fd_, IPPROTO_TCP, 
        TCP_NODELAY, (const char*)&optval, sizeof(optval))) {
    LOG_ERR("setsockopt error err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::SetReuseAddr(bool reuse) 
{
  if (kNetTypeInvalid == fd_)
    return false;

  int optval = (reuse ? 1 : 0);
  if (kNetTypeError == setsockopt(fd_, SOL_SOCKET, 
        SO_REUSEADDR, (const char*)&optval, sizeof(optval))) {
    LOG_ERR("setsockopt error err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::SetKeepAlive(bool keep)
{
  if (kNetTypeInvalid == fd_)
    return false;

  int optval = (keep ? 1 : 0);
  if (kNetTypeError == setsockopt(fd_, SOL_SOCKET, 
        SO_KEEPALIVE, (const char*)&optval, sizeof(optval))) {
    LOG_ERR("setsockopt error err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::SetReadBuffer(int bytes) 
{
  if (kNetTypeInvalid == fd_)
    return false;

  if (kNetTypeError == setsockopt(fd_, SOL_SOCKET, 
        SO_RCVBUF, (const char*)&bytes, sizeof(bytes))) {
    LOG_ERR("setsockopt error err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::SetWriteBuffer(int bytes) 
{
  if (kNetTypeInvalid == fd_)
    return false;

  if (kNetTypeError == setsockopt(fd_, SOL_SOCKET, 
        SO_SNDBUF, (const char*)&bytes, sizeof(bytes))) {
    LOG_ERR("setsockopt error err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::SetNonBlock(void)
{
  if (kNetTypeInvalid == fd_)
    return false;

  u_long val = 1;
  if (kNetTypeError == ioctlsocket(fd_, FIONBIO, &val)) {
    LOG_ERR("ioctlsocket error err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::Open(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (kNetTypeInvalid == fd_) {
    LOG_FAIL("socket failed err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

void 
Socket::Close(void)
{
  if (kNetTypeInvalid != fd_) {
    shutdown(fd_, SD_BOTH);
    closesocket(fd_);
    fd_ = kNetTypeInvalid;
  }
}

bool 
Socket::Bind(const char* ip, unsigned short port)
{
  if (kNetTypeInvalid == fd_)
    return false;

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = 
    (NULL == ip ? htonl(INADDR_ANY) : inet_addr(ip));
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);

  if (kNetTypeError == bind(fd_, 
        (struct sockaddr*)&host_addr, sizeof(host_addr))) {
    LOG_FAIL("bind failed err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::Listen(void)
{
  if (kNetTypeInvalid == fd_)
    return false;

  if (kNetTypeError == listen(fd_, SOMAXCONN)) {
    LOG_FAIL("listen failed err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

bool 
Socket::Accept(Socket* s, struct sockaddr* remote_addr)
{
  if (kNetTypeInvalid == fd_ || NULL == s)
    return false;

  int addrlen = sizeof(struct sockaddr_in);
  int fd = accept(fd_, remote_addr, &addrlen);
  if (kNetTypeInvalid == fd) 
    return false;

  s->Attach(fd);
  return true;
}

bool 
Socket::Connect(const char* ip, unsigned short port)
{
  if (kNetTypeInvalid == fd_)
    return false;

  if (NULL == ip)
    ip = "127.0.0.1";
  struct sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr(ip);
  remote_addr.sin_family      = AF_INET;
  remote_addr.sin_port        = htons(port);

  if (kNetTypeError == connect(fd_, 
        (struct sockaddr*)&remote_addr, sizeof(remote_addr))) {
    LOG_FAIL("connect failed err-code(%d)\n", NetErrno());
    return false;
  }

  return true;
}

int 
Socket::Read(int length, char* buffer)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  int ret = recv(fd_, buffer, length, 0);
  if (kNetTypeError == ret)
    LOG_ERR("recv error err-code(%d)\n", NetErrno());

  return ret;
}

int 
Socket::Write(const char* buffer, int length)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  int total = 0;
  int ret;
  while (total < length) {
    ret = send(fd_, buffer + total, length - total, 0);
    if (kNetTypeError == ret) {
      LOG_ERR("send error err-code(%d)\n", NetErrno());
      return ret;
    }

    total += ret;
  }

  return total;
}
