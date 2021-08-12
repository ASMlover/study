// Copyright (c) 2021 ASMlover. All rights reserved.
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
#include <iostream>

// region ---------------------------------- vftable helper functions ------------------------------
inline void* get_vfptr(void* obj) noexcept {
  return (void*)*(unsigned long*)obj;
}

inline void* get_vftable(void* obj) noexcept {
  return (void*)((unsigned long*)obj)[0];
}

inline void* get_virtual_func_addr(void* obj, unsigned int offset = 0) noexcept {
  unsigned long* vftable_addr = (unsigned long*)((unsigned long*)obj)[0];
  return (void*)vftable_addr[offset];
}

inline void print_class_vftable(const char* class_name, void* obj, int vfcount = 1) noexcept {
  std::cout
    << "class `" << class_name << "` <" << obj << ">" << std::endl
    << "+--- __vfptr                | " << get_vfptr(obj) << std::endl
    << "         +--- vftable       | " << get_vftable(obj) << std::endl;
  for (int i = 0; i < vfcount; ++i)
    std::cout << "                 +--- vf[" << i << "] | " << get_virtual_func_addr(obj, i) << std::endl;
  std::cout << std::endl;
}
// endregion ------------------------------- vftable helper functions ------------------------------

// region ---------------------------------- vftable test demo class -------------------------------
class Base {
public:
  int foo_{1};

  virtual void print(int x = 2) {
    std::cout << "Base::print - " << foo_ + x << std::endl;
  }
};

class Derived : public Base {
public:
  int bar_{3};

  virtual void print(int x = 10) override {
    std::cout << "Derived::print - " << bar_ + x << std::endl;
  }
};
// endregion ------------------------------- vftable test demo class -------------------------------

class Parent {
public:
  virtual void f() { std::cout << "Parent::f()" << std::endl; }
  virtual void g() { std::cout << "Parent::g()" << std::endl; }
  virtual void h() { std::cout << "Parent::h()" << std::endl; }
};

class Children : public Parent {
public:
  virtual void f() override { std::cout << "Children::f()" << std::endl; }
  virtual void g() override { std::cout << "Children::g()" << std::endl; }
  virtual void h() override { std::cout << "Children::h()" << std::endl; }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  {
    Base* pb = new Derived;
    pb->print();

    print_class_vftable("Derived", pb, 1);
    delete pb;
  }

  {
    Parent* p = new Children;
    print_class_vftable("Children", p, 3);

    typedef void (*pvfun)();

    pvfun f = (pvfun)get_virtual_func_addr(p, 0);
    pvfun g = (pvfun)get_virtual_func_addr(p, 1);
    pvfun h = (pvfun)get_virtual_func_addr(p, 2);
    f();
    g();
    h();

    delete p;
  }

  return 0;
}
