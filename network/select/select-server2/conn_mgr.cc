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
#include "common.h"
#include "socket.h"
#include "conn_mgr.h"
#include "event_handler.h"


ConnectorMgr::ConnectorMgr(void)
{
}

ConnectorMgr::~ConnectorMgr(void)
{
}

void 
ConnectorMgr::Insert(int fd, int ev)
{
  Socket* s = new Socket();
  if (NULL == s) {
    LOG_ERR("new Socket error ...\n");
    return;
  }
  s->Attach(fd);

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
  if (it == connectors_.end())
    connectors_[fd] = std::make_pair<int, Socket*>(ev, s);
}

void 
ConnectorMgr::Remove(int fd)
{
  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
  if (it != connectors_.end()) {
    it->second.second->Close();
    delete it->second.second;
    connectors_.erase(it);
  }
}

void 
ConnectorMgr::AddEvent(int fd, int ev)
{
  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
  if (it != connectors_.end())
    it->second.first |= ev;
}

void 
ConnectorMgr::DelEvent(int fd, int ev)
{
  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  it = connectors_.find(fd);
  if (it != connectors_.end())
    it->second.first &= ~ev;
}

Socket* 
ConnectorMgr::GetConnector(int fd)
{
  Socket* s = NULL;

  {
    LockerGuard<SpinLock> guard(spinlock_);
    std::map<int, std::pair<int, Socket*> >::iterator it;
    it = connectors_.find(fd);
    if (it != connectors_.end())
      s = it->second.second;
  }

  return s;
}

void 
ConnectorMgr::InitSelectSets(fd_set* rset, fd_set* wset)
{
  FD_ZERO(rset);
  FD_ZERO(wset);

  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, std::pair<int, Socket*> >::iterator it;
  for (it = connectors_.begin(); it != connectors_.end();) {
    if (INVALID_SOCKET == it->second.second->fd()) {
      it = connectors_.erase(it);
    }
    else {
      if (it->second.first & EventHandler::kEventTypeRead)
        FD_SET(it->first, rset);

      if (it->second.first & EventHandler::kEventTypeWrite)
        FD_SET(it->first, wset);
    }
  }
}
