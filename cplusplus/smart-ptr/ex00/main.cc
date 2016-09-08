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
#include <iostream>
#include <memory>
#include <vector>
#include "uncopyable.h"

#define UNUSED(x) {x = x;}

class Parent;
typedef std::shared_ptr<Parent> ParentPtr;

class Child : private UnCopyable {
  std::weak_ptr<Parent> mon_;
  std::weak_ptr<Parent> dad_;
public:
  explicit Child(const ParentPtr& mon, const ParentPtr& dad)
    : mon_(mon)
    , dad_(dad) {
  }
};
typedef std::shared_ptr<Child> ChildPtr;

class Parent : private UnCopyable {
  std::weak_ptr<Parent> spouser_;
  std::vector<ChildPtr> children_;
public:
  Parent(void) {
  }

  void SetSpouser(const ParentPtr& spouser) {
    spouser_ = spouser;
  }

  void AddChild(const ChildPtr& child) {
    children_.push_back(child);
  }

  void ShowChild(void) {
    std::cout << "length of `children_` is: " << children_.size() << std::endl;
  }
};

int main(int argc, char* argv[]) {
  UNUSED(argc)
  UNUSED(argv)

  ParentPtr mon(new Parent());
  ParentPtr dad(new Parent());
  mon->SetSpouser(dad);
  dad->SetSpouser(mon);
  mon->ShowChild();
  dad->ShowChild();

  {
    ChildPtr child(new Child(mon, dad));
    mon->AddChild(child);
    dad->AddChild(child);
  }
  mon->ShowChild();
  dad->ShowChild();

  {
    ChildPtr child(new Child(mon, dad));
    mon->AddChild(child);
    dad->AddChild(child);
  }
  mon->ShowChild();
  dad->ShowChild();

  return 0;
}
