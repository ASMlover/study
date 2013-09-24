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
#include "buffer_file.h"



struct Data {
  int a, b, c;
};

#define LOOP_TIMES  (1000000)


int 
main(int argc, char* argv[])
{
  int counter;
  DWORD beg, end;
  BufferFile bf;
  char buffer[1024 * 16];

  Data d = {1, 2, 3};
  
  if (bf.Open("demo.txt")) {
    counter = 0;
    beg = GetTickCount();
    while (counter++ < LOOP_TIMES) 
      bf.Write(&d, sizeof(d));
    end = GetTickCount();
    fprintf(stdout, "use self buffer: %lu\n", end - beg);
  }
  bf.Close();


  FILE* fp = fopen("demo1.txt", "w");
  setvbuf(fp, buffer, _IOFBF, sizeof(buffer));
  counter = 0;
  beg = GetTickCount();
  while (counter++ < LOOP_TIMES)
    fwrite(&d, sizeof(d), 1, fp);
  end = GetTickCount();
  fprintf(stdout, "use stdio: %lu\n", end - beg);
  fclose(fp);

  return 0;
}
