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
# define EWOULDBLOCK  WSAWOULDBLOCK
# define NErrno()     WSAGetLastError()
#elif defined(EV_POSIX)
# include <errno.h>
# define NErrno()     errno
#endif
#include <stdio.h>
#include "socket.h"



EventHandler::~EventHandler(void)
{
}

bool 
EventHandler::AcceptEvent(Socket* s)
{
  return true;
}

void 
EventHandler::CloseEvent(Socket* s)
{
}

bool 
EventHandler::ReadEvent(Socket* s)
{
  return true;
}







EventDispatcher::EventDispatcher(void)
  : handler_(NULL)
{
}

EventDispatcher::~EventDispatcher(void)
{
}

bool 
EventDispatcher::DispatchReader(Socket* s)
{
  if (NULL == handler_ || NULL == s)
    return false;

  int read_bytes = s->DealWithAsyncRead();
  if (read_bytes > 0) {
    //! TODO:
    //! solve full package
    handler_->ReadEvent(s);
  }
  else if (0 == read_bytes) {
    //! TODO:
    //! remove fd in poll 
    handler_->CloseEvent(s);
    s->Close();

    //! TODO:
    //! remove in connector manager
  }
  else {
    if (EWOULDBLOCK != NErrno()) {
      //! TODO:
      //! remove fd in poll 
      handler_->CloseEvent(s);
      s->Close();

      //! TODO:
      //! remove in connector manager
    }
  }

  return true;
}

bool 
EventDispatcher::DispatchWriter(Socket* s)
{
  if (NULL == handler_ || NULL == s)
    return false;

  int write_bytes = s->DealWithAsyncWrite();
  if (write_bytes > 0) {
    //! TODO:
  }

  return true;
}
