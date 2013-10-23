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
#include "listener.h"
#include "worker.h"
#include "conn_mgr.h"
#include "network.h"


Network::Network(void)
  : conn_mgr_(NULL)
  , worker_(NULL)
  , listener_(NULL)
  , event_handler_(NULL)
{
}

Network::~Network(void)
{
  Destroy();
}

void 
Network::Attach(EventHandler* handler)
{
  event_handler_ = handler;
}

void 
Network::Init(void)
{
  if (NULL == event_handler_)
    return;

  conn_mgr_ = new ConnectorMgr();
  if (NULL == conn_mgr_)
    LOG_FAIL("new ConnectorMgr failed ...\n");

  worker_ = new Worker();
  if (NULL == worker_)
    LOG_FAIL("new Worker failed ...\n");
  worker_->Attach(conn_mgr_);
  worker_->SetEventHandler(event_handler_);

  worker_->Start();
}

void 
Network::Destroy(void)
{
  if (NULL != listener_) {
    listener_->Stop();
    delete listener_;
    listener_ = NULL;
  }

  if (NULL != worker_) {
    worker_->Stop();
    delete worker_;
    worker_ = NULL;
  }

  if (NULL != conn_mgr_) {
    conn_mgr_->CloseAll();
    delete conn_mgr_;
    conn_mgr_ = NULL;
  }
}

void 
Network::Listen(const char* ip, unsigned short port)
{
  if (NULL == conn_mgr_)
    return;

  listener_ = new Listener();
  if (NULL == listener_)
    LOG_FAIL("new Listener failed ...\n");
  listener_->Attach(conn_mgr_);

  listener_->Start(ip, port);
}
