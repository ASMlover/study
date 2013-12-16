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
#include "libnet_internal.h"
#if defined(PLATFORM_WIN)
# ifndef _WINDOWS_
#   include <winsock2.h>
# endif
# define EAGAIN     WSAEWOULDBLOCK
# define NErrno()   WSAGetLastError()
#elif defined(PLATFORM_POSIX)
# include <errno.h>
# define NErrno()   errno
#endif
#include "socket.h"
#include "connector.h"
#include "connector_dispatcher.h"


ConnectorDispatcher::ConnectorDispatcher(void)
  : rbytes_(kDefaultBufferLen)
  , wbytes_(kDefaultBufferLen)
  , spinlock_()
  , handler_(NULL)
{
}

ConnectorDispatcher::~ConnectorDispatcher(void)
{
  CloseAll();
}

void 
ConnectorDispatcher::CloseAll(void)
{
  std::map<int, Connector*>::iterator it;
  for (it = connectors_.begin(); it != connectors_.end(); ++it) {
    if (NULL != it->second) {
      it->second->Close();
      delete it->second;
    }
  }
  connectors_.clear();
}

Connector* 
ConnectorDispatcher::Insert(int fd)
{
  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Connector*>::iterator it = connectors_.find(fd);
  if (it == connectors_.end()) {
    Connector* conn = new Connector();
    if (NULL == conn)
      return NULL;

    conn->Attach(fd);
    conn->SetNonBlock();
    conn->SetTcpNoDelay();
    conn->SetReuseAddr();
    conn->SetKeepAlive();
    conn->SetReadBuffer(rbytes_);
    conn->SetWriteBuffer(wbytes_);

    connectors_[fd] = conn;

    return conn;
  }

  return NULL;
}

void 
ConnectorDispatcher::Remove(int fd)
{
  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Connector*>::iterator it = connectors_.find(fd);
  if (it != connectors_.end()) {
    if (NULL != it->second) {
      it->second->Close();
      delete it->second;
    }

    connectors_.erase(it);
  }
}


bool 
ConnectorDispatcher::DispatchReader(Poller* poller, Connector* conn)
{
  if (NULL == handler_ || NULL == poller || NULL == conn)
    return false;

  int read_bytes = conn->DealWithAsyncRead();
  if (read_bytes > 0) {
    handler_->ReadEvent(conn);
  }
  else {
    if (0 == read_bytes || EAGAIN != NErrno()) {
      poller->Remove(conn);

      handler_->CloseEvent(conn);

      Remove(conn->fd());
    }
  }

  return true;
}

bool 
ConnectorDispatcher::DispatchWriter(Poller* poller, Connector* conn)
{
  if (NULL == poller || NULL == conn)
    return false;

  int write_bytes = conn->DealWithAsyncWrite();
  if (write_bytes > 0) {
  }
  else {
    if (0 == write_bytes || EAGAIN != NErrno()) {
      poller->Remove(conn);

      handler_->CloseEvent(conn);

      Remove(conn->fd());
    }
  }

  return true;
}
