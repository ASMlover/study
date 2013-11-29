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
#include "select_ev.h"
#if defined(EV_WIN)
# ifndef _WINDOWS_
#   include <winsock2.h>
# endif
#elif defined(EV_POSIX)
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <unistd.h>
# include <fcntl.h>
#endif
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
#if defined(EV_WIN)
        (const char*)&optval, 
#elif defined(EV_POSIX)
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

#if defined(EV_WIN)
  u_long val = 1;
  if (kNetTypeInval == ioctlsocket(fd_, FIONBIO, &val))
    return false;
#elif defined(EV_POSIX)
  int optval = fcntl(fd_, F_GETFL);
  if (kNetTypeError == optval)
    return false;
  if (kNetTypeError == fcntl(fd_, F_SETFL, optval | O_NONBLOCK))
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
#if defined(EV_WIN)
    shutdown(fd_, SD_BOTH);
    closesocket(fd_);
#elif defined(EV_POSIX)
    shutdown(fd_, SHUT_RDWR);
    close(fd_);
#endif

    fd_ = kNetTypeInval;
  }
}
