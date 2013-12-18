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
#include "worker.h"
#include "listener.h"
#include "connector_dispatcher.h"
#include "network.h"



Network::Network(void)
  : dispatcher_(NULL)
  , worker_count_(kDefaultWorkerCount)
  , workers_(NULL)
  , listener_(NULL)
  , handler_(NULL)
{
}

Network::~Network(void)
{
  Destroy();
}


bool 
Network::Init(int worker_count, uint32_t rbytes, uint32_t wbytes)
{
  if (NULL == handler_)
    return false;

  dispatcher_ = new ConnectorDispatcher();
  if (NULL == dispatcher_)
    return false;
  dispatcher_->Attach(handler_);
  dispatcher_->SetBuffer(rbytes, wbytes);

  worker_count_ = (worker_count > kDefaultWorkerCount ? 
      worker_count : kDefaultWorkerCount);

  do {
    workers_ = new Worker[worker_count_];
    if (NULL == workers_)
      break;

    for (int i = 0; i < worker_count_; ++i) {
      workers_[i].Attach(dispatcher_);
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
  if (NULL != listener_) {
    listener_->Stop();

    delete listener_;
    listener_ = NULL;
  }

  if (NULL != workers_) {
    for (int i = 0; i < worker_count_; ++i)
      workers_[i].Stop();

    delete [] workers_;
    workers_ = NULL;
  }
  worker_count_ = kDefaultWorkerCount;

  if (NULL != dispatcher_) {
    dispatcher_->CloseAll();

    delete dispatcher_;
    dispatcher_ = NULL;
  }

}

bool 
Network::Listen(const char* ip, uint16_t port)
{
  if (NULL == dispatcher_)
    return false;

  listener_ = new Listener();
  if (NULL == listener_)
    return false;
  listener_->Attach(dispatcher_);

  if (!listener_->Start(ip, port)) {
    delete listener_;
    listener_ = NULL;

    return false;
  }

  return true;
}
