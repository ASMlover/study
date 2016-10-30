// Copyright (c) 2016 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <ucontext.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../tpp_corotine.h"

#define COROTINE_STACKSIZE  (1024 * 1024)

namespace tpp {


class __libtpp_context_t {
public:
  ucontext_t uc_;
  CorotineCallback closure_{nullptr};
  void* arg_{nullptr};
  CoStatus status_{CoStatus::DEAD};
  char* stack_{nullptr};

  __libtpp_context_t(const CorotineCallback& cb, void* arg, bool is_main = false)
    : closure_(cb)
    , arg_(arg)
    , status_(CoStatus::READY) {
    if (!is_main) {
      int pagesz = getpagesize();
      char* buf = (char*)malloc(COROTINE_STACKSIZE + pagesz);
      mprotect(buf, pagesz, PROT_NONE);
      stack_ = buf + pagesz;
    }
  }

  ~__libtpp_context_t(void) {
    if (nullptr != stack_) {
      int pagesz = getpagesize();
      char* buf = stack_ - pagesz;
      mprotect(buf, pagesz, PROT_READ | PROT_WRITE);
      free(buf);
    }
  }
};

//  std::unique_ptr<__libtpp_context_t> main_;
//  std::weak_ptr<__libtpp_context_t> running_;
//  std::set<__libtpp_context_ptr> co_;

Corotine::Corotine(void)
  : main_(new __libtpp_context_t(nullptr, nullptr, true)) {
  main_->status_ = CoStatus::RUNNING;
  running_ = main_;
}

Corotine::~Corotine(void) {
  co_.clear();
}

__libtpp_context_ptr Corotine::create(const CorotineCallback& cb, void* arg) {
  __libtpp_context_ptr c(new __libtpp_context_t(cb, arg));
  co_.insert(c);
  return c;
}

__libtpp_context_ptr Corotine::create(CorotineCallback&& cb, void* arg) {
  __libtpp_context_ptr c(new __libtpp_context_t(std::move(cb), arg));
  co_.insert(c);
  return c;
}

void Corotine::closure_callback(Corotine* c) {
  auto co = c->running_.lock();
  if (co) {
    if (co->closure_)
      co->closure_(co->arg_);
    co->status_ = CoStatus::DEAD;
    c->running_.reset();
  }
}

bool Corotine::resume(__libtpp_context_ptr& c) {
  auto iter = co_.find(c);
  if (iter == co_.end())
    return false;

  switch (c->status_) {
  case CoStatus::READY:
    getcontext(&c->uc_);
    c->uc_.uc_stack.ss_sp = c->stack_;
    c->uc_.uc_stack.ss_size = COROTINE_STACKSIZE;
    c->uc_.uc_link = &main_->uc_;
    running_ = c;
    c->status_ = CoStatus::RUNNING;
    makecontext(&c->uc_, (void(*)(void))&Corotine::closure_callback, 1, this);

    swapcontext(&main_->uc_, &c->uc_);
    break;
  case CoStatus::SUSPEND:
    running_ = c;
    c->status_ = CoStatus::RUNNING;
    swapcontext(&main_->uc_, &c->uc_);
    break;
  default:
    return false;
  }

  if (c->status_ == CoStatus::DEAD)
    co_.erase(co_.find(c));

  return true;
}

bool Corotine::yield(void) {
  __libtpp_context_ptr c = running_.lock();
  if (c == main_)
    return false;

  c->status_ = CoStatus::SUSPEND;
  swapcontext(&c->uc_, &main_->uc_);

  return true;
}

CoStatus Corotine::status(__libtpp_context_ptr& c) {
  return c ? c->status_ : CoStatus::DEAD;
}

__libtpp_context_ptr Corotine::running(void) {
  return running_.lock();
}

}
