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
#include "el_socket.h"
#include "el_connector.h"
#include "el_connector_dispatcher.h"


namespace el {


ConnectorDispatcher::ConnectorDispatcher(void)
  : rbytes_(kDefBufferSize)
  , wbytes_(kDefBufferSize)
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
}

Connector* 
ConnectorDispatcher::Insert(int fd)
{
  return NULL;
}

void 
ConnectorDispatcher::Remove(int fd)
{
}

bool 
ConnectorDispatcher::DispatchReader(Poller* poller, Connector* conn)
{
  return true;
}

bool 
ConnectorDispatcher::DispatchWriter(Poller* poller, Connector* conn)
{
  return true;
}


}
