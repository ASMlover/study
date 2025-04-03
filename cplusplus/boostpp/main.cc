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
#include <iostream>

extern void boost_scoped_ptr() noexcept;
extern void boost_scoped_array() noexcept;
extern void boost_shared_ptr() noexcept;
extern void boost_make_shared() noexcept;
extern void boost_shared_array() noexcept;
extern void boost_weak_ptr() noexcept;
extern void boost_ptr_vector() noexcept;
extern void boost_ptr_set() noexcept;
extern void boost_scope_exit() noexcept;
extern void boost_pool() noexcept;
extern void boost_string_algorithms() noexcept;
extern void boost_lexical_cast() noexcept;
extern void boost_format() noexcept;
extern void boost_regex() noexcept;
extern void boost_xpressive() noexcept;
extern void boost_tokenizer() noexcept;
extern void boost_spirit() noexcept;
extern void boost_multiindex() noexcept;
extern void boost_bimap() noexcept;
extern void boost_array() noexcept;

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  std::cout << "Hello, Boost !!!" << std::endl;

  boost_scoped_ptr();
  boost_scoped_array();
  boost_shared_ptr();
  boost_make_shared();
  boost_shared_array();
  boost_weak_ptr();
  boost_ptr_vector();
  boost_ptr_set();
  boost_scope_exit();
  boost_pool();
  boost_string_algorithms();
  boost_lexical_cast();
  boost_format();
  boost_regex();
  boost_xpressive();
  boost_tokenizer();
  // boost_spirit();
  boost_multiindex();
  boost_bimap();
  boost_array();

  return 0;
}
