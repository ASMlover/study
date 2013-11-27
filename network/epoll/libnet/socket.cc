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
#if defined(_WINDOWS_) || defined(_MSC_VER)
# ifndef _WINDOWS_
#   include <winsock2.h>
# endif
  typedef int socklen_t;
#elif defined(__linux__)
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <unistd.h>
# include <fcntl.h>
#endif
#include "net.h"
#include "address.h"
#include "socket.h"



Socket::Socket(void)
  : fd_(kNetTypeInval)
{
}

Socket::~Socket(void)
{
  Close();
}

bool 
Socket::SetOption(int level, int optname, int optval)
{
  if (kNetTypeInval == fd_)
    return false;

  if (kNetTypeError == setsockopt(fd_, level, 
        optname, 
#if defined(_WINDOWS_) || defined(_MSC_VER)
        (const char*)&optval, 
#elif defined(__linux__)
        (const void*)&optval, 
#endif
        sizeof(optval)))
    return false;

  return true;
}

bool 
Socket::SetNonBlock(void)
{
  if (kNetTypeInval == fd_)
    return false;

#if defined(_WINDOWS_) || defined(_MSC_VER)
  u_long mode = 1;
  if (kNetTypeError == ioctlsocket(fd_, FIONBIO, &mode))
    return false;
#elif defined(__linux__)
  int opt = fcntl(fd_, F_GETFL);
  if (kNetTypeError == fd_)
    return false;
  if (kNetTypeError == fcntl(fd_, F_SETFL, opt | O_NONBLOCK))
    return false;
#endif

  return true;
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
Socket::SetReadBuffer(int bytes)
{
  return SetOption(SOL_SOCKET, SO_RCVBUF, bytes);
}

bool 
Socket::SetWriteBuffer(int bytes)
{
  return SetOption(SOL_SOCKET, SO_SNDBUF, bytes);
}

bool 
Socket::SetSelfReadBuffer(int bytes)
{
  return rbuf_.Init(bytes);
}

bool 
Socket::SetSelfWriteBuffer(int bytes)
{
  return wbuf_.Init(bytes);
}


bool 
Socket::Open(void)
{
  fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (kNetTypeInval == fd_)
    return false;

  return true;
}

void 
Socket::Close(void)
{
  if (kNetTypeInval != fd_) {
#if defined(_WINDOWS_) || defined(_MSC_VER)
    shutdown(fd_, SD_BOTH);
    closesocket(fd_);
#elif defined(__linux__)
    shutdown(fd_, SHUT_RDWR);
    close(fd_);
#endif
    fd_ = kNetTypeInval;
  }
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
Socket::ReadBlock(int bytes, char* buffer)
{
  if (kNetTypeInval == fd_ || bytes <= 0 || NULL == buffer)
    return kNetTypeError;

  int ret = recv(fd_, buffer, bytes, 0);

  return ret;
}

int 
Socket::WriteBlock(const char* buffer, int bytes)
{
  if (kNetTypeInval == fd_ || NULL == buffer || bytes <= 0)
    return kNetTypeError;

  int write_bytes = 0;
  int ret;
  while (write_bytes < bytes) {
    ret = send(fd_, buffer + write_bytes, bytes - write_bytes, 0);
    if (kNetTypeError == ret)
      return ret;

    write_bytes += ret;
  }

  return write_bytes;
}

int 
Socket::Read(int bytes, char* buffer)
{
  if (kNetTypeInval == fd_ || bytes <= 0 || NULL == buffer)
    return kNetTypeError;

  return rbuf_.Get(bytes, buffer);
}

int 
Socket::Write(const char* buffer, int bytes)
{
  if (kNetTypeInval == fd_ || NULL == buffer || bytes <= 0)
    return kNetTypeError;

  return wbuf_.Put(buffer, bytes);
}

int 
Socket::DealWithAsyncRead(void)
{
  if (kNetTypeInval == fd_)
    return kNetTypeError;

  int read_bytes = 0;
  int ret;
  char* free_buffer = rbuf_.free_buffer();
  int   free_length = rbuf_.free_length();
  while (true) {
    if (0 == free_length) {
      rbuf_.Regrow();
      free_buffer = rbuf_.free_buffer();
      free_length = rbuf_.free_length();
    }

    ret = recv(fd_, free_buffer, free_length, 0);
    if (ret > 0) {
      rbuf_.Increment(ret);
      read_bytes += ret;
      if (ret < free_length)
        break;
    }
    else if (0 == ret) {
      if (read_bytes > 0)
        break;
      else 
        return ret;
    }
    else {
      return ret;
    }
  }

  return read_bytes;
}

int 
Socket::DealWithAsyncWrite(void)
{
  if (kNetTypeInval == fd_)
    return kNetTypeError;

  int write_bytes = 0;
  while (true) {
    int length = wbuf_.length();
    if (length <= 0)
      return 0;

    const char* buffer = wbuf_.buffer();
    int ret = send(fd_, buffer, length, 0);

    if (ret > 0) {
      wbuf_.Decrement(ret);
      write_bytes += ret;
      if (ret < length)
        break;
    }
    else if (0 == ret) {
      if (write_bytes > 0)
        break;
      else 
        return ret;
    }
    else {
      return ret;
    }
  }

  return write_bytes;
}

bool 
Socket::CheckValidMessageInReadBuffer(void)
{
  return true;
}
