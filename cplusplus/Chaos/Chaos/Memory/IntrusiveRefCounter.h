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
#ifndef CHAOS_MEMORY_INTRUSIVEREFCOUNTER_H
#define CHAOS_MEMORY_INTRUSIVEREFCOUNTER_H

#include <stdint.h>
#include <atomic>

namespace Chaos {

struct CountedThreadUnsafe {
  typedef uint32_t type;

  static uint32_t load(uint32_t counter) {
    return counter;
  }

  static void increment(uint32_t& counter) {
    ++counter;
  }

  static uint32_t decrement(uint32_t& counter) {
    return --counter;
  }
};

struct CountedThreadSafe {
  typedef std::atomic<uint32_t> type;

  static uint32_t load(const std::atomic<uint32_t>& counter) {
    return static_cast<uint32_t>(counter);
  }

  static void increment(std::atomic<uint32_t>& counter) {
    ++counter;
  }

  static uint32_t decrement(std::atomic<uint32_t>& counter) {
    return static_cast<uint32_t>(--counter);
  }
};

template <typename DerivedT, typename CounterPolicyT>
class IntrusiveRefCounter;

template <typename DerivedT, typename CounterPolicyT>
void intrusive_ptr_add_ref(const IntrusiveRefCounter<DerivedT, CounterPolicyT>* p);
template <typename DerivedT, typename CounterPolicyT>
void intrusive_ptr_del_ref(const IntrusiveRefCounter<DerivedT, CounterPolicyT>* p);

template <typename DerivedT, typename CounterPolicyT>
class IntrusiveRefCounter {
  typedef typename CounterPolicyT::type counter_type;
  mutable counter_type ref_counter_{};
public:
  IntrusiveRefCounter& operator=(const IntrusiveRefCounter&) {
    return *this;
  }

  uint32_t use_count(void) const {
    return CounterPolicyT::load(ref_counter_);
  }
protected:
  template <typename _Derived, typename _CounterPolicy>
  friend void intrusive_ptr_add_ref(const IntrusiveRefCounter<_Derived, _CounterPolicy>* p);
  template <typename _Derived, typename _CounterPolicy>
  friend void intrusive_ptr_del_ref(const IntrusiveRefCounter<_Derived, _CounterPolicy>* p);
};

template <typename DerivedT, typename CounterPolicyT>
inline void intrusive_ptr_add_ref(const IntrusiveRefCounter<DerivedT, CounterPolicyT>* p) {
  CounterPolicyT::increment(p->ref_counter_);
}

template <typename DerivedT, typename CounterPolicyT>
inline void intrusive_ptr_del_ref(const IntrusiveRefCounter<DerivedT, CounterPolicyT>* p) {
  CounterPolicyT::decrement(p->ref_counter_);
}

}

#endif // CHAOS_MEMORY_INTRUSIVEREFCOUNTER_H
