// Copyright (c) 2014 ASMlover. All rights reserved.
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
#include <utility.h>

class Value : private util::UnCopyable {
  int val_;
public:
  explicit Value(int v) 
    : val_(v) {
    fprintf(stdout, "%s => val_ = %d\n", __func__, val_);
  }

  ~Value(void) {
    fprintf(stdout, "%s => val_ = %d\n", __func__, val_);
  }
};


class TraceLock : private util::UnCopyable {
public:
  TraceLock(void) {
    fprintf(stdout, "%s\n", __func__);
  }

  ~TraceLock(void) {
    fprintf(stdout, "%s\n", __func__);
  }

  void Lock(void) {
    fprintf(stdout, "%s\n", __func__);
  }

  void Unlock(void) {
    fprintf(stdout, "%s\n", __func__);
  }
};

static inline void CloseFile(FILE* p) {
  fprintf(stdout, "%s\n", __func__);
  fclose(p);
}

int main(int argc, char* argv[]) {
  {
    util::SmartPtr<Value, TraceLock> v(new Value(23));
    v.Reset(new Value(33));
  }

  {
    util::SmartPtr<FILE, TraceLock> f(fopen("demo.txt", "w"), CloseFile);
    f.Reset(fopen("demo1.txt", "w"), CloseFile);
    util::SmartPtr<FILE, TraceLock> f1(fopen("demo2.txt", "w"), CloseFile);
    f1 = f;
  }


  return 0;
}
