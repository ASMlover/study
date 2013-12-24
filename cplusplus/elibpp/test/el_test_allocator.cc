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
#include "el_test_header.h"
#include "../el_allocator.h"




#define ALLOC_TIMES   (10000)
#define LOOP_TIMES    (500)



UNIT_IMPL(Allocator)
{
  int* arr[ALLOC_TIMES];
  int counter;
  clock_t beg, end;

  counter = 0;
  beg = clock();
  while (counter++ < LOOP_TIMES) {
    for (int i = 0; i < ALLOC_TIMES; ++i) {
      arr[i] = (int*)malloc(sizeof(int));

      free(arr[i]);
    }
  }
  end = clock();
  fprintf(stdout, "\t\tdefault use : %lu\n", end - beg);
  
  counter = 0;
  beg = clock();
  while (counter++ < LOOP_TIMES) {
    for (int i = 0; i < ALLOC_TIMES; ++i) {
      arr[i] = (int*)el::NEW(sizeof(int));

      el::DEL(arr[i]);
    }
  }
  end = clock();
  fprintf(stdout, "\t\tAllocator use : %lu\n", end - beg);
}



class Person : public el::SmallAllocator {
public:
  int age_;
  int sex_;
public:
  explicit Person(int age = 20, int sex = 0)
    : age_(age)
    , sex_(sex)
  {
  }
};

UNIT_IMPL(SmallAllocator)
{
  Person* p = new Person(22, 1);
  UNIT_ASSERT(NULL != p);
  delete p;
}
