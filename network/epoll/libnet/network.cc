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
#include "net.h"
#include "logging.h"
#if defined(_WINDOWS_) || defined(_MSC_VER)
# include "select_poll.h"
#elif defined(__linux__)
# include "epoll_poll.h"
#endif
#include "worker.h"
#include "listener.h"
#include "network.h"



Network::Network(void)
  : handler_(NULL)
  , poll_(NULL)
  , workers_(NULL)
  , worker_count_(kDefaultWorkerCount)
{
  listeners_.clear();
}

Network::~Network(void)
{
  Destroy();
}

bool 
Network::Init(int worker_count, int rbytes, int wbytes)
{
  if (NULL == handler_)
    return false;

#if defined(_WINDOWS_) || defined(_MSC_VER)
  poll_ = new SelectPoll();
#elif defined(__linux__)
  poll_ = new EpollPoll();
#endif
  if (NULL == poll_)
    return false;

  poll_->Attach(handler_);
  poll_->SetBuffer(rbytes, wbytes);

  worker_count_ = (worker_count > kDefaultWorkerCount 
      ? worker_count : kDefaultWorkerCount);
  workers_ = new Worker[worker_count_];
  if (NULL == workers_) {
    LOG_FAIL("new Worker failed\n");
    return false;
  }

  for (int i = 0; i < worker_count_; ++i) {
    workers_[i].Attach(poll_);
    workers_[i].Start();
  }

  return true;
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

  if (NULL != poll_) {
    poll_->CloseAll();

    delete poll_;
    poll_ = NULL;
  }
}

bool 
Network::Listen(const char* ip, unsigned short port)
{
  if (NULL == poll_ || NULL == handler_)
    return false;

  Listener* listener = new Listener();
  if (NULL == listener)
    return false;

  listener->Attach(poll_);
  listener->Attach(handler_);

  if (!listener->Start(ip, port)) {
    delete listener;
    return false;
  }
  
  listeners_.push_back(listener);
  
  return true;
}
