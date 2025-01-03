// Copyright (c) 2024 ASMlover. All rights reserved.
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
#include <boost/scope_exit.hpp>
#include <iostream>
#include <utility>

static int* foo() {
  int* i = new int{10};

  BOOST_SCOPE_EXIT(&i) {
    delete i;
    i = nullptr;
  } BOOST_SCOPE_EXIT_END
  std::cout << "[demo.scope_exit] " << *i << std::endl;
  return i;
}

template <typename T> struct scope_exit {
  T t_;

  scope_exit(T&& t) noexcept
    : t_{std::move(t)} {
  }

  ~scope_exit() noexcept {
    t_();
  }
};

template <typename T> scope_exit<T> make_scope_exit(T&& t) noexcept {
  return scope_exit<T>{std::move(t)};
}

static int* foo2() noexcept {
  int* i = new int{10};
  auto cleanup = make_scope_exit([&i]() mutable {
    delete i;
    i = nullptr;
  });

  std::cout << "[demo.scope_exit] " << *i << std::endl;
  return i;
}

void boost_scope_exit() noexcept {
  std::cout << "========= [scope_exit] =========" << std::endl;

  int* j = foo();
  std::cout << "[demo.scope_exit] " << j << std::endl;

  std::cout << "--------- [scope_exit.make_scope_exit] ---------" << std::endl;
  int* j2 = foo2();
  std::cout << "[demo.scope_exit] " << j2 << std::endl;
}
