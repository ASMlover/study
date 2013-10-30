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
#elif defined(__linux__)
# include <sys/types.h>
# include <sys/select.h>
# include <unistd.h>
#endif
#include "net.h"
#include "logging.h"
#include "socket.h"
#include "select_poll.h"




SelectPoll::SelectPoll(void)
  : rbytes_(kDefaultBuffer)
  , wbytes_(kDefaultBuffer)
  , handler_(NULL)
  , spinlock_()
{
  connectors_.clear();
  FD_ZERO(&rset_);
  FD_ZERO(&wset_);
}

SelectPoll::~SelectPoll(void)
{
  CloseAll();
}

void 
SelectPoll::CloseAll(void)
{
}

bool 
SelectPoll::Insert(int fd, int ev)
{
  return true;
}

void 
SelectPoll::Remove(int fd)
{
}

bool 
SelectPoll::AddEvent(int fd, int ev)
{
  return true;
}

bool 
SelectPoll::DelEvent(int fd, int ev)
{
  return true;
}

Socket* 
SelectPoll::GetConnector(int fd)
{
  Socket* s = NULL;
  
  return s;
}

bool 
SelectPoll::Polling(int millitm)
{
  return true;
}

void 
SelectPoll::InitSets(int* max_fd)
{
}

#if defined(_WINDOWS_) || defined(_MSC_VER)
void 
SelectPoll::DispatchEvent(fd_set* set, int ev)
{
}
#elif defined(__linux__)
void 
SelectPoll::DispatchEvents(void)
{
}
#endif
