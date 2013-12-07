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
#include <string.h>
#include "socket.h"
#include "connector.h"
#include "connector_mgr.h"



ConnectorMgr::ConnectorMgr(void)
  : worker_count_(kDefaultWorkerCount)
  , rbytes_(kDefaultBufferSize)
  , wbytes_(kDefaultBufferSize)
  , worker_connectors_(NULL)
{
  connectors_.clear();
}

ConnectorMgr::~ConnectorMgr(void)
{
  Destroy();
}


bool 
ConnectorMgr::Init(int worker_count, int rbytes, int wbytes)
{
  worker_count_ = (worker_count > kDefaultWorkerCount ? 
      worker_count : kDefaultWorkerCount);

  rbytes_ = rbytes;
  wbytes_ = wbytes;

  size_t size = sizeof(int) * worker_count_;
  worker_connectors_ = (int*)malloc(size);
  if (NULL == worker_connectors_)
    return false;

  memset(worker_connectors_, 0, size);
  return true;
}

void 
ConnectorMgr::Destroy(void)
{
  CloseAll();

  if (NULL != worker_connectors_) {
    free(worker_connectors_);
    worker_connectors_ = NULL;
  }
  worker_count_ = kDefaultWorkerCount;
}

int 
ConnectorMgr::SuitableWorker(void)
{
  if (kDefaultWorkerCount == worker_count_)
    return 0;

  int worker_id = 0;
  int min_worker = worker_connectors_[0];
  for (int i = 0; i < worker_count_; ++i) {
    if (0 == worker_connectors_[i]) {
      worker_id = i;
      break;
    }

    if (worker_connectors_[i] < min_worker) {
      min_worker = worker_connectors_[i];
      worker_id = i;
    }
  }

  return worker_id;
}


void 
ConnectorMgr::CloseAll(void)
{
  LockerGuard<SpinLock> guard(spinlock_);
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
ConnectorMgr::Insert(int fd, int worker_id)
{
  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Connector*>::iterator it = connectors_.find(fd);
  if (it == connectors_.end()) {
    Connector* conn = new Connector();
    if (NULL == conn)
      return NULL;

    conn->Attach(fd);
    conn->Attach(worker_id);
    conn->SetNonBlock();
    conn->SetTcpNoDelay();
    conn->SetReuseAddr();
    conn->SetKeepAlive();
    conn->SetSelfReadBuffer(rbytes_);
    conn->SetSelfWriteBuffer(wbytes_);

    connectors_[fd] = conn;

    ++worker_connectors_[worker_id];

    return conn;
  }

  return NULL;
}

void 
ConnectorMgr::Remove(int fd)
{
  LockerGuard<SpinLock> guard(spinlock_);
  std::map<int, Connector*>::iterator it = connectors_.find(fd);
  if (it != connectors_.end()) {
    if (NULL != it->second) {
      --worker_connectors_[it->second->worker_id()];
      it->second->Close();
      delete it->second;
    }
    connectors_.erase(it);
  }
}
