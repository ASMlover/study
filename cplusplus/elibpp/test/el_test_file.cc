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
#include "../el_file.h"


#define LOOP_TIMES  (100000)


UNIT_IMPL(File)
{
  const char* s = "Hello, world! el::File testing ...\n";
  size_t      n = strlen(s);
  int counter;
  clock_t beg, end;

  FILE* fp = fopen("demo1.log", "a+");
  setvbuf(fp, NULL, _IOFBF, 16 * 1024);

  counter = 0;
  beg = clock();
  while (counter++ < LOOP_TIMES) {
#if defined(_WINDOWS_) || defined(_MSC_VER)
    _fwrite_nolock(s, sizeof(char), n, fp);
#elif defined(__linux__)
    fwrite_unlocked(s, sizeof(char), n, fp);
#endif 
  }
  end = clock();
  fprintf(stdout, "\t\tbuffered IO use:\t%lu\n", end - beg);

  fclose(fp);



  el::File f;
  f.Open("demo2.log", true);
  
  counter = 0;
  beg = clock();
  while (counter++ < LOOP_TIMES)
    f.Write(s, n);
  end = clock();
  fprintf(stdout, "\t\tel::File IO use:\t%lu\n", end - beg);

  f.Close();
}
