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
#include <stdlib.h>
#include <time.h>

#include "special_pool.h"



struct object1_t {
  int a; 
  int b;
  int c;
};

struct object2_t : public special_pool_t<object2_t> {
  int a;
  int b;
  int c;
};

template<>
free_space_t* special_pool_t<object2_t>::free_space_ = 0;



int 
main(int argc, char* argv[])
{
#define ALLOC_TIMES   (100000)
#define LOOP_TIMES    (500)

  object1_t* arr1[ALLOC_TIMES];
  object2_t* arr2[ALLOC_TIMES];
  clock_t beg, end;

  object2_t::init();


  beg = clock();
  for (int l = 0; l < LOOP_TIMES; ++l) {
    for (int i = 0; i < ALLOC_TIMES; ++i) {
      arr1[i] = new object1_t();

      delete arr1[i];
    }
  }
  end = clock();
  fprintf(stdout, "use default memory allocator: %u\n", end - beg);
  
  beg = clock();
  for (int l = 0; l < LOOP_TIMES; ++l) {
    for (int i = 0; i < ALLOC_TIMES; ++i) {
      arr2[i] = new object2_t();

      delete arr2[i];
    }
  }
  end = clock();
  fprintf(stdout, "use special pool: %u\n", end - beg);


  object2_t::destroy();

  return 0;
}
