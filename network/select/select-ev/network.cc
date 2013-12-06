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
#include "connector_mgr.h"
#include "worker.h"
#include "listener.h"
#include "network.h"



Network::Network(void)
  : worker_count_(kDefaultWorkerCount)
  , workers_(NULL)
  , conn_mgr_(NULL)
  , handler_(NULL)
{
}

Network::~Network(void)
{
}

bool 
Network::Init(int worker_count, int rbytes, int wbytes)
{
  if (NULL == handler_)
    return false;

  worker_count_ = (worker_count > kDefaultWorkerCount ? 
      worker_count : kDefaultWorkerCount);

  conn_mgr_ = new ConnectorMgr();
  if (NULL == conn_mgr_)
    return false;
  conn_mgr_->Init(worker_count_, rbytes, wbytes);

  do {
    workers_ = new Worker[worker_count_];
    if (NULL == workers_)
      break;
    for (int i = 0; i < worker_count_; ++i) {
      workers_[i].Attach(i);
      workers_[i].Attach(conn_mgr_);
      workers_[i].Attach(handler_);

      workers_[i].Start();
    }

    return true;
  } while (0);

  Destroy();
  return false;
}

void 
Network::Destroy(void)
{
  std::vector<Listener*>::iterator it;
  for (it = listeners_.begin(); it != listeners_.end(); ++it) {
    if (NULL != *it) {
      (*it)->Stop();

      delete *it;
    }
  }
  listeners_.clear();

  if (NULL != workers_) {
    for (int i = 0; i < worker_count_; ++i)
      workers_[i].Stop();

    delete [] workers_;
    workers_ = NULL;
  }

  if (NULL != conn_mgr_) {
    conn_mgr_->CloseAll();

    delete conn_mgr_;
    conn_mgr_ = NULL;
  }

  worker_count_ = kDefaultWorkerCount;
}


bool 
Network::Listen(const char* ip, unsigned short port)
{
  if (NULL == conn_mgr_)
    return false;

  Listener* listener = new Listener();
  if (NULL == listener)
    return false;
  listener->Attach(conn_mgr_);

  if (!listener->Start(ip, port)) {
    delete listener;
    return false;
  }
  
  listeners_.push_back(listener);
  return true;
}
