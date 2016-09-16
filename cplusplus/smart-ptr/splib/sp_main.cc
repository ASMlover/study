// Copyright (c) 2016 ASMlover. All rights reserved.
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
#include "sp_smart_ptr.h"

class Person : sp::UnCopyable {
  std::string name_;
  int age_;
public:
  Person(const std::string& name, int age)
    : name_(name)
    , age_(age) {
  }

  void Display(void) {
    std::cout << "{name => `" << name_ << "`, age => " << age_ << "}" << std::endl;
  }

  const char* GetName(void) const {
    return name_.c_str();
  }

  int GetAge(void) const {
    return age_;
  }
};

std::ostream& operator<<(std::ostream& cout, const Person& p) {
  return cout << "{name => `" << p.GetName() << "`, age => " << p.GetAge() << "}";
}

void sp_SmartPtrTest(void) {
  sp::SmartPtr<Person> p(new Person("Jack.Tomy", 33));
  p->Display();
  std::cout << *p << std::endl;
}

int main(int argc, char* argv[]) {
  UNUSED(argc)
  UNUSED(argv)

  sp_SmartPtrTest();

  return 0;
}
