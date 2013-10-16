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
#include <winsock2.h>
#include "global.h"
#include "thread.h"
#include "select.h"
#include "network.h"
#include "thread_listener.h"
#include "thread_worker.h"


Network::Network(void)
  : select_(NULL)
  , listener_(NULL)
  , worker_(NULL)
{
}

Network::~Network(void)
{
}

void 
Network::Init(EventHandler* (*getHandler)(Socket*))
{
  select_ = new Select();
  if (NULL == select_)
    LOG_FAIL("new Select failed ...\n");

  listener_ = new ThreadListener();
  if (NULL == listener_) 
    LOG_FAIL("new ThreadListener failed ...\n");
  listener_->Attach(select_, getHandler);

  worker_ = new ThreadWorker();
  if (NULL == worker_)
    LOG_FAIL("new ThreadWorker failed ...\n");
  worker_->Attach(select_);

  worker_->Start();
}

void 
Network::Start(const char* ip, unsigned short port)
{
  fprintf(stdout, "%s\n", __FUNCSIG__);
  listener_->Start(ip, port);
}

void 
Network::Stop(void)
{
  if (NULL != worker_) {
    worker_->Stop();
    delete worker_;
    worker_ = NULL;
  }
  if (NULL != listener_) {
    listener_->Stop();
    delete listener_;
    listener_ = NULL;
  }
  if (NULL != select_) {
    delete select_;
    select_ = NULL;
  }
}

