// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __ESTL_ALLOC_HEADER_H__
#define __ESTL_ALLOC_HEADER_H_

namespace estl {

class Alloc {
  enum Align { ALIGN = 8 };
  enum MaxBytes { MAXBYTES = 128 };
  enum FreeLists { NFREELISTS = (MaxBytes::MAXBYTES / Align::ALIGN) };
  enum Objs { NOBJS = 20 };

  union Obj {
    union Objs* next;
    char        client[1];
  };
  static Obj* free_list_[FreeLists::NFREELISTS];
  static char*  start_free_;  // start position of memory pool
  static char*  finish_free_; // finish position of memory pool
  static size_t heap_size_;
private:
  static size_t ROUND_UP(size_t bytes) {
    return ((bytes + Align::ALIGN - 1) & ~(Align::ALIGN - 1));
  }

  static size_t FREELIST_INDEX(size_t bytes) {
    return ((bytes + Align::ALIGN - 1) / Align::ALIGN - 1);
  }

  static void* ReFill(size_t n);
  static char* ChunkAlloc(size_t n, size_t& nobjs);
public:
  static void* Allocate(size_t bytes);
  static void* Reallocate(void* ptr, size_t old_bytes, size_t new_bytes);
  static void Deallocate(void* ptr, size_t bytes);
};

}

#endif  // __ESTL_ALLOC_HEADER_H_
