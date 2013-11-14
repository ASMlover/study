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
#include <stdlib.h>
#include "net.h"
#include "select_poll.h"
#include "select_worker.h"
#include "select_listener.h"
#include "select_network.h"




SelectNetwork::SelectNetwork(void)
  : handler_(NULL)
  , poll_(NULL)
  , workers_(NULL)
  , worker_count_(kDefaultWorkerCount)
{
  listeners_.clear();
}

SelectNetwork::~SelectNetwork(void)
{
  Destroy();
}

bool 
SelectNetwork::Init(int worker_count, int rbytes, int wbytes)
{
  if (NULL == handler_)
    return false;

  poll_ = new SelectPoll();
  if (NULL == poll_) {
    abort();
    return false;
  }
  poll_->Attach(handler_);
  poll_->SetBuffer(rbytes, wbytes);

  worker_count_ = (worker_count > kDefaultWorkerCount ?
      worker_count : kDefaultWorkerCount);
  workers_ = new SelectWorker[worker_count_];
  if (NULL == workers_) {
    abort();
    return false;
  }
  for (int i = 0; i < worker_count_; ++i) {
    workers_[i].Attach(poll_);
    workers_[i].Start();
  }

  return true;
}

void 
SelectNetwork::Destroy(void)
{
  std::vector<SelectListener*>::iterator it;
  for (it = listeners_.begin(); it != listeners_.end(); ++it) {
    if (NULL != (*it)) {
      (*it)->Stop();
      delete (*it);
    }
  }
  listeners_.clear();

  if (NULL != workers_) {
    for (int i = 0; i < worker_count_; ++i)
      workers_[i].Stop();

    delete [] workers_;
    workers_ = NULL;
  }
  worker_count_ = kDefaultWorkerCount;

  if (NULL != poll_) {
    poll_->CloseAll();
    delete poll_;
    poll_ = NULL;
  }
}



bool 
SelectNetwork::Listen(const char* ip, unsigned short port)
{
  if (NULL == handler_ || NULL == poll_)
    return false;

  SelectListener* listener = new SelectListener();
  if (NULL == listener) {
    abort();
    return false;
  }

  listener->Attach(handler_);
  listener->Attach(poll_);
  if (listener->Start(ip, port)) {
    listeners_.push_back(listener);

    return true;
  }

  return false;
}
