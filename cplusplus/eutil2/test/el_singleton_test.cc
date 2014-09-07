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
#include "el_test.h"

class SingleInteger : public el::Singleton<SingleInteger> {
  std::string name_;
  int         value_;
public:
  SingleInteger(void) 
    : name_("SingleInteger")
    , value_(0) {
  }

  inline int Increment(void) {
    return ++value_;
  }

  inline std::string name(void) const {
    return std::string(name_);
  }

  inline int value(void) const {
    return value_;
  }
};

static void SingleFunction1(void) {
  UNIT_PRINT("%s\n", __func__);
  
  UNIT_PRINT("%s : %d\n", 
      SingleInteger::Instance().name().c_str(), 
      SingleInteger::Instance().value());
  SingleInteger::Instance().Increment();
  UNIT_PRINT("%s : %d\n", 
      SingleInteger::Instance().name().c_str(), 
      SingleInteger::Instance().value());
}

static void SingleFunction2(void) {
  UNIT_PRINT("%s\n", __func__);
  
  UNIT_PRINT("%s : %d\n", 
      SingleInteger::Instance().name().c_str(), 
      SingleInteger::Instance().value());
  SingleInteger::Instance().Increment();
  UNIT_PRINT("%s : %d\n", 
      SingleInteger::Instance().name().c_str(), 
      SingleInteger::Instance().value());
}

UNIT_IMPL(Singleton) {
  SingleFunction1();
  SingleFunction2();
}
