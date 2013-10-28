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
#include "address.h"
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
Socket::SetSockOpt(int level, int optname, int optval) 
{
  if (kNetTypeInvalid == fd_)
    return false;

  if (kNetTypeError == setsockopt(fd_, level, 
        optname, (const char*)&optval, sizeof(optval))) {
    NLOG_ERRX("setsockopt error\n");
    return false;
  }

  return true;
}

bool 
Socket::SetTcpNoDelay(bool nodelay)
{
  return SetSockOpt(IPPROTO_TCP, TCP_NODELAY, (nodelay ? 1 : 0));
}

bool 
Socket::SetReuseAddr(bool reuse) 
{
  return SetSockOpt(SOL_SOCKET, SO_REUSEADDR, (reuse ? 1 : 0));
}

bool 
Socket::SetKeepAlive(bool keep)
{
  return SetSockOpt(SOL_SOCKET, SO_KEEPALIVE, (keep ? 1 : 0));
}

bool 
Socket::SetReadBuffer(int bytes) 
{
  return SetSockOpt(SOL_SOCKET, SO_RCVBUF, bytes);
}

bool 
Socket::SetWriteBuffer(int bytes) 
{
  return SetSockOpt(SOL_SOCKET, SO_SNDBUF, bytes);
}

bool 
Socket::SetNonBlock(void)
{
  if (kNetTypeInvalid == fd_)
    return false;

  u_long val = 1;
  if (kNetTypeError == ioctlsocket(fd_, FIONBIO, &val)) {
    NLOG_ERRX("ioctlsocket error\n");
    return false;
  }

  return true;
}

void 
Socket::SetSelfReadBuffer(int bytes)
{
  rbuf_.Init(bytes);
}

void 
Socket::SetSelfWriteBuffer(int bytes)
{
  wbuf_.Init(bytes);
}

bool 
Socket::Open(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (kNetTypeInvalid == fd_) {
    NLOG_FAILX("socket failed\n");
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
    NLOG_FAILX("bind failed\n");
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
    NLOG_FAILX("listen failed\n");
    return false;
  }

  return true;
}

bool 
Socket::Accept(Socket* s, Address* remote_addr)
{
  if (kNetTypeInvalid == fd_ || NULL == s)
    return false;

  struct sockaddr_in addr;
  int addrlen = sizeof(addr);
  int fd = accept(fd_, (struct sockaddr*)&addr, &addrlen);
  if (kNetTypeInvalid == fd) 
    return false;

  s->Attach(fd);
  if (NULL != remote_addr)
    remote_addr->Attach(&addr);
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
    NLOG_FAILX("connect failed\n");
    return false;
  }

  return true;
}

int 
Socket::Read(int length, char* buffer)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  return rbuf_.Get(length, buffer);
}

int 
Socket::Write(const char* buffer, int length)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  return wbuf_.Put(buffer, length);
}

int 
Socket::ReadBlock(int length, char* buffer)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  int ret = recv(fd_, buffer, length, 0);
  if (kNetTypeError == ret)
    NLOG_ERRX("recv error\n");

  return ret;
}

int 
Socket::WriteBlock(const char* buffer, int length)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  int total = 0;
  int ret;
  while (total < length) {
    ret = send(fd_, buffer + total, length - total, 0);
    if (kNetTypeError == ret) {
      NLOG_ERRX("send error\n");
      return ret;
    }

    total += ret;
  }

  return total;
}

int 
Socket::DealWithRead(void)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  char* free_space = rbuf_.free_space();
  int free_length = rbuf_.free_length();

  if (0 == free_length) {
    rbuf_.Regrow();
    free_space = rbuf_.free_space();
    free_length = rbuf_.free_length();
  }

  int ret = recv(fd_, free_space, free_length, 0);
  if (ret > 0)
    rbuf_.Increment(ret);

  return ret;
}

int 
Socket::DealWithWrite(void)
{
  if (kNetTypeInvalid == fd_)
    return kNetTypeError;

  int length = wbuf_.length();
  if (length <= 0)
    return 0;

  const char* buffer = wbuf_.buffer();
  int ret = send(fd_, buffer, length, 0);

  if (ret > 0)
    wbuf_.Decrement(ret);

  return ret;
}
