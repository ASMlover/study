//! Copyright (c) 2014 ASMlover. All rights reserved.
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


class Base {
public:
  explicit Base(void) {
    fprintf(stdout, "Base::Base\n");
  }

  virtual ~Base(void) {
    fprintf(stdout, "Base::~Base\n");
  }

  virtual void Show(void) {
    fprintf(stdout, "Base::Show\n");
  }
};

class Child : public Base {
public:
  explicit Child(void) {
    fprintf(stdout, "Child::Child\n");
  }

  ~Child(void) {
    fprintf(stdout, "Child::~Child\n");
  }

  virtual void Show(void) {
    fprintf(stdout, "Child::Show\n");
  }
};


static void 
ConstCast(const void* data)
{
  const char* s = static_cast<const char*>(const_cast<void*>(data));
  fprintf(stdout, "%s\n", s);
}

static void 
DynamicCast(Base& b) 
{
  Child& c = dynamic_cast<Child&>(b);
  c.Show();
}


int 
main(int argc, char* argv[])
{
  if (argc < 2)
    return 0;

  ConstCast(argv[1]);

  Child c;
  DynamicCast(c);

  return 0;
}
