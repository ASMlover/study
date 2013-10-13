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
#include <assert.h>
#include "global.h"
#include "socket.h"



Socket::Socket(void)
  : fd_(INVALID_SOCKET)
  , blocked_(true)
{
}

Socket::~Socket(void)
{
}

void 
Socket::SetTcpNoDelay(bool nodelay)
{
  int optval = (nodelay ? 1 : 0);
  if (SOCKET_ERROR == setsockopt(fd_, IPPROTO_TCP, 
        TCP_NODELAY, (const char*)&optval, sizeof(optval)))
    LOG_ERR("setsockopt error err-code (%d)\n", WSAGetLastError());
}

void 
Socket::SetReuseAddr(bool reuse) 
{
  int optval = (reuse ? 1 : 0);
  if (SOCKET_ERROR == setsockopt(fd_, SOL_SOCKET, 
        SO_REUSEADDR, (const char*)&optval, sizeof(optval)))
    LOG_ERR("setsockopt error err-code (%d)\n", WSAGetLastError());
}

void 
Socket::SetKeepAlive(bool keep) 
{
  int optval = (keep ? 1 : 0);
  if (SOCKET_ERROR == setsockopt(fd_, SOL_SOCKET, 
        SO_KEEPALIVE, (const char*)&optval, sizeof(optval)))
    LOG_ERR("setsockopt error err-code (%d)\n", WSAGetLastError());
}

void 
Socket::SetSendBuffer(int size)
{
  if (SOCKET_ERROR == setsockopt(fd_, SOL_SOCKET, 
        SO_SNDBUF, (const char*)&size, sizeof(size)))
    LOG_ERR("setsockopt error err-code (%d)\n", WSAGetLastError());
}

void 
Socket::SetRecvBuffer(int size)
{
  if (SOCKET_ERROR == setsockopt(fd_, SOL_SOCKET, 
        SO_RCVBUF, (const char*)&size, sizeof(size)))
    LOG_ERR("setsockopt error err-code (%d)\n", WSAGetLastError());
}

void 
Socket::SetBlocking(bool blocked)
{
  unsigned long val;
  if (blocked) {
    val = 0;
    if (SOCKET_ERROR == ioctlsocket(fd_, FIONBIO, &val)) {
      LOG_ERR("ioctlsocket error err-code (%d)\n", WSAGetLastError());
      return;
    }

    blocked_ = true;
  }
  else {
    val = 1;
    if (SOCKET_ERROR == ioctlsocket(fd_, FIONBIO, &val)) {
      LOG_ERR("ioctlsocket error err-code (%d)\n", WSAGetLastError());
      return;
    }

    blocked_ = false;
  }
}




void 
Socket::Open(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == fd_)
    LOG_FAIL("socket failed err-code (%d)\n", WSAGetLastError());
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
    LOG_FAIL("bind failed err-code (%d)\n", WSAGetLastError());
}

void 
Socket::Listen(void)
{
  if (SOCKET_ERROR == listen(fd_, SOMAXCONN))
    LOG_FAIL("listen failed err-code (%d)\n", WSAGetLastError());
}

void 
Socket::Close(void) 
{
  shutdown(fd_, SD_BOTH);
  closesocket(fd_);
  fd_ = INVALID_SOCKET;
}

int 
Socket::Accept(struct sockaddr* addr)
{
  assert(blocked_);

  struct sockaddr_in remote_addr;
  int addrlen = sizeof(remote_addr);

  int s = accept(fd_, 
      (NULL != addr ? addr : (struct sockaddr*)&remote_addr), &addrlen);
  if (INVALID_SOCKET == s) 
    LOG_ERR("accept failed err-code (%d)\n", WSAGetLastError());

  return s;
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
    LOG_ERR("connect failed err-code (%d)\n", WSAGetLastError());
    return false;
  }

  return true;
}

int 
Socket::Read(int length, char* buffer)
{
  assert(blocked_);

  int ret = recv(fd_, buffer, length, 0);
  if (SOCKET_ERROR == ret)
    LOG_ERR("recv error err-code (%d)\n", WSAGetLastError());

  return ret;
}

void 
Socket::Write(const char* buffer, int length)
{
  assert(blocked_);

  int total = 0;
  int ret = 0;
  while (total < length) {
    ret = send(fd_, buffer + total, length - total, 0);
    if (SOCKET_ERROR == ret) {
      LOG_ERR("send error err-code (%d)\n", WSAGetLastError());
      return;
    }

    total += ret;
  }
}
