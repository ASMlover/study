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
#ifndef __EL_NET_INTERNAL_HEADER_H__
#define __EL_NET_INTERNAL_HEADER_H__

#include "el_config.h"

#if defined(USING_SELECT)
# ifndef _WINDOWS_
#   include <winsock2.h>
# endif
  typedef int socklen_t;
# define EAGAIN     WSAEWOULDBLOCK
# define NERROR()   WSAGetLastError()
#elif defined(USING_EPOLL)
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# define NERROR()   errno
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
#include <algorithm>

#include "elib_internal.h"
#include "el_net.h"
#include "el_net_buffer.h"

namespace el {


struct Poller;
struct Dispatcher {
  virtual ~Dispatcher(void) {}
  virtual bool DispatchReader(Poller* poller, Connector* conn) = 0;
  virtual bool DispatchWriter(Poller* poller, Connector* conn) = 0;
};


struct ConnectorHolder {
  virtual ~ConnectorHolder(void) {}
  virtual void CloseAll(void) = 0;
  virtual Connector* Insert(int fd) = 0;
  virtual void Remove(int fd) = 0;
};


struct Poller {
  virtual ~Poller(void) {}
  virtual bool Insert(Connector* conn) = 0;
  virtual void Remove(Connector* conn) = 0;
  virtual bool AddEvent(Connector* conn, int ev) = 0;
  virtual bool DelEvent(Connector* conn, int ev) = 0;
  virtual bool Dispatch(Dispatcher* dispatcher, uint32_t millitm) = 0;
};


}

#endif  //! __EL_NET_INTERNAL_HEADER_H__
