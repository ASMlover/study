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
#include "el_net_internal.h"
#include "el_address.h"
#include "el_socket.h"




namespace el {



Socket::Socket(void)
  : fd_(kNetTypeInval)
{
}

Socket::~Socket(void)
{
  Close();
}


bool 
Socket::SetTcpNoDelay(bool nodelay)
{
  return SetOption(IPPROTO_TCP, TCP_NODELAY, (nodelay ? 1 : 0));
}

bool 
Socket::SetReuseAddr(bool reuse)
{
  return SetOption(SOL_SOCKET, SO_REUSEADDR, (reuse ? 1 : 0));
}

bool 
Socket::SetKeepAlive(bool keep)
{
  return SetOption(SOL_SOCKET, SO_KEEPALIVE, (keep ? 1 : 0));
}


bool 
Socket::Open(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (kNetTypeInval == fd_)
    return false;

  return true;
}

bool 
Socket::Bind(const char* ip, unsigned short port)
{
  if (kNetTypeInval == fd_)
    return false;

  struct sockaddr_in host_addr;
  host_addr.sin_addr.s_addr = 
    (NULL == ip ? htonl(INADDR_ANY) : inet_addr(ip));
  host_addr.sin_family      = AF_INET;
  host_addr.sin_port        = htons(port);

  if (kNetTypeError == bind(fd_, 
        (struct sockaddr*)&host_addr, sizeof(host_addr)))
    return false;

  return true;
}

bool 
Socket::Listen(void)
{
  if (kNetTypeInval == fd_)
    return false;

  if (kNetTypeError == listen(fd_, SOMAXCONN))
    return false;

  return true;
}

bool 
Socket::Accept(Socket* s, Address* addr)
{
  if (kNetTypeInval == fd_ || NULL == s)
    return false;

  struct sockaddr_in remote_addr;
  socklen_t addrlen = sizeof(remote_addr);
  int fd = accept(fd_, (struct sockaddr*)&remote_addr, &addrlen);
  if (kNetTypeInval == fd)
    return false;

  s->Attach(fd);
  if (NULL != addr)
    addr->Attach(&remote_addr);

  return true;
}

bool 
Socket::Connect(const char* ip, unsigned short port)
{
  if (kNetTypeInval == fd_)
    return false;

  if (NULL == ip)
    ip = "127.0.0.1";
  struct sockaddr_in remote_addr;
  remote_addr.sin_addr.s_addr = inet_addr(ip);
  remote_addr.sin_family      = AF_INET;
  remote_addr.sin_port        = htons(port);
  if (kNetTypeError == connect(fd_, 
        (struct sockaddr*)&remote_addr, sizeof(remote_addr)))
    return false;

  return true;
}

int 
Socket::Recv(int bytes, char* buffer)
{
  if (kNetTypeInval == fd_ || bytes <= 0 || NULL == buffer)
    return kNetTypeError;

  return recv(fd_, buffer, bytes, 0);
}

int 
Socket::Send(const char* buffer, int bytes)
{
  if (kNetTypeInval == fd_ || NULL == buffer || bytes <= 0)
    return kNetTypeError;

  return send(fd_, buffer, bytes, 0);
}


}
