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
#include "el_connector_dispatcher.h"
#include "el_net_worker.h"
#include "el_net_listener.h"
#include "el_network_handler.h"


namespace el {


NetworkHandler::NetworkHandler(void)
  : dispatcher_(NULL)
  , worker_count_(kDefaultWorkerCount)
  , workers_(NULL)
  , listener_(NULL)
  , suitable_worker_(0)
  , handler_(NULL)
{
}

NetworkHandler::~NetworkHandler(void)
{
  Destroy();
}

bool 
NetworkHandler::Init(int worker_count, uint32_t rbuf, uint32_t wbuf)
{
  if (NULL == handler_)
    return false;

  if (NULL == (dispatcher_ = new ConnectorDispatcher()))
    return false;
  dispatcher_->Attach(handler_);
  dispatcher_->SetBuffer(rbuf, wbuf);

  worker_count_ = (worker_count > kDefaultWorkerCount ? 
      worker_count : kDefaultWorkerCount);

  do {
    if (NULL == (workers_ = new NetWorker[worker_count_]))
      break;

    for (int i = 0; i < worker_count_; ++i) {
      workers_[i].Attach(dispatcher_);
      workers_[i].Start();
    }

    return true;
  } while (0);

  Destroy();
  return true;
}

void 
NetworkHandler::Destroy(void)
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

NetWorker& 
NetworkHandler::SuitableWorker(void)
{
  return workers_[suitable_worker_];
}

void 
NetworkHandler::MarkNextSuitableWorker(void)
{
  suitable_worker_ = (suitable_worker_ + 1) % worker_count_;
}


bool 
NetworkHandler::Listen(const char* ip, uint16_t port)
{
  if (NULL == dispatcher_)
    return false;

  if (NULL == (listener_ = new NetListener()))
    return false;
  listener_->Attach(this);
  listener_->Attach(dispatcher_);

  if (!listener_->Start(ip, port)) {
    delete listener_;
    listener_ = NULL;

    return false;
  }

  return true;
}


}
