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
#include "../el_net_internal.h"
#include "../el_socket.h"
#include "../el_connector.h"
#include "el_win_select.h"



namespace el {


struct win_fd_set {
  u_int fd_count;
  int fd_array[1];
};



struct SelectEntry {
  int fd;
  Connector* conn;
};



Select::Select(void)
  : fd_storage_(FD_SETSIZE)
  , rset_in_(NULL)
  , wset_in_(NULL)
  , rset_out_(NULL)
  , wset_out_(NULL)
  , has_removed_(false)
{
}

Select::~Select(void)
{
}

bool 
Select::Init(void)
{
  return false;
}

void 
Select::Destroy(void)
{
}

bool 
Select::Regrow(void)
{
  return true;
}


bool 
Select::Insert(Connector* conn)
{
  return true;
}

void 
Select::Remove(Connector* conn)
{
}

bool 
Select::AddEvent(Connector* conn, int ev)
{
  return true;
}

bool 
Select::DelEvent(Connector* conn, int ev)
{
  return true;
}

bool 
Select::Dispatch(Dispatcher* dispatcher, uint32_t millitm)
{
  return true;
}


}
