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
#ifndef TPP_COROTINE_H_
#define TPP_COROTINE_H_

#include <functional>
#include <memory>
#include <set>
#include "tpp_types.h"

namespace tpp {

enum class CoStatus : int {
  DEAD = 0,
  READY,
  RUNNING,
  SUSPEND,
};


typedef std::function<void (void*)> CorotineCallback;

class __libtpp_context_t;
typedef std::shared_ptr<__libtpp_context_t> __libtpp_context_ptr;
class Corotine : private UnCopyable {
  __libtpp_context_ptr main_;
  std::weak_ptr<__libtpp_context_t> running_;
  std::set<__libtpp_context_ptr> co_;
private:
  static void closure_callback(Corotine* c);
public:
  Corotine(void);
  ~Corotine(void);

  __libtpp_context_ptr create(const CorotineCallback& cb, void* arg);
  __libtpp_context_ptr create(CorotineCallback&& cb, void* arg);

  bool resume(__libtpp_context_ptr& c);
  bool yield(void);
  CoStatus status(__libtpp_context_ptr& c);
  __libtpp_context_ptr running(void);
};

}

#endif // TPP_COROTINE_H_
