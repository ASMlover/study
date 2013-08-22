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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "memory_pool.h"



class test_data_t {
  int id_;
  char name_[32];
public:
  test_data_t(int id, const char* name)
  {
    id_ = id;
    strcpy(name_, name);
  }
};

class test_object_t : public test_data_t {
  static memory_pool_t pool_;
public:
  test_object_t(int id, const char* name) 
    : test_data_t(id, name)
  {
  }

  inline void* 
  operator new(size_t size)
  {
    return pool_.alloc(size);
  }

  inline void 
  operator delete(void* ptr, size_t size)
  {
    pool_.dealloc(ptr);
  }
};
memory_pool_t test_object_t::pool_;


int 
main(int argc, char* argv[])
{
  const int ALLOC_TIMES = 10000, LOOP_TIMES = 500;
  int count;
  clock_t beg, end;
  test_data_t*    arr1[ALLOC_TIMES];
  test_object_t*  arr2[ALLOC_TIMES];


  count = 0;
  beg = clock();
  while (count++ < LOOP_TIMES) {
    for (int i = 0; i < ALLOC_TIMES; ++i) {
      arr1[i] = new test_data_t(i, "default");

      delete arr1[i];
    }
  }
  end = clock();
  fprintf(stdout, "default use : %lu\n", end - beg);
  
  
  count = 0;
  beg = clock();
  while (count++ < LOOP_TIMES) {
    for (int i = 0; i < ALLOC_TIMES; ++i) {
      arr2[i] = new test_object_t(i, "memory.pool");

      delete arr2[i];
    }
  }
  end = clock();
  fprintf(stdout, "memory.pool use : %lu\n", end - beg);

  return 0;
}
