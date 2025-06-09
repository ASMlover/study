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
#include <unordered_map>
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
extern void boost_unordered() noexcept;
extern void boost_circular_buffer() noexcept;
extern void boost_heap() noexcept;
extern void boost_intrusive() noexcept;
extern void boost_multi_array() noexcept;
extern void boost_container() noexcept;
extern void boost_optional() noexcept;
extern void boost_tuple() noexcept;
extern void boost_any() noexcept;
extern void boost_variant() noexcept;
extern void boost_property_tree() noexcept;

typedef void (*FuncPtr)();
static std::unordered_map<std::string, FuncPtr> kBoostFuncs = {
  {"scoped_ptr",          boost_scoped_ptr},
  {"scoped_array",        boost_scoped_array},
  {"shared_ptr",          boost_shared_ptr},
  {"make_shared",         boost_make_shared},
  {"shared_array",        boost_shared_array},
  {"weak_ptr",            boost_weak_ptr},
  {"ptr_vector",          boost_ptr_vector},
  {"ptr_set",             boost_ptr_set},
  {"scope_exit",          boost_scope_exit},
  {"pool",                boost_pool},
  {"string_algorithms",   boost_string_algorithms},
  {"lexical_cast",        boost_lexical_cast},
  {"format",              boost_format},
  {"regex",               boost_regex},
  {"xpressive",           boost_xpressive},
  {"tokenizer",           boost_tokenizer},
  {"spirit",              boost_spirit},
  {"multiindex",          boost_multiindex},
  {"bimap",               boost_bimap},
  {"array",               boost_array},
  {"unordered",           boost_unordered},
  {"circular_buffer",     boost_circular_buffer},
  {"heap",                boost_heap},
  {"intrusive",           boost_intrusive},
  {"multi_array",         boost_multi_array},
  {"container",           boost_container},
  {"optional",            boost_optional},
  {"tuple",               boost_tuple},
  {"any",                 boost_any},
  {"variant",             boost_variant},
  {"ptree",               boost_property_tree},
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  std::cout << "Hello, Boost !!!" << std::endl;


  if (kBoostFuncs.empty())
    return 0;
  const char* func_key = nullptr;
  if (argc < 2) {
    auto it = kBoostFuncs.end();
    func_key = (--it)->first.c_str();
  }
  else {
    func_key = argv[1];
  }

  if (auto it = kBoostFuncs.find(func_key); it != kBoostFuncs.end())
    it->second();

  return 0;
}
