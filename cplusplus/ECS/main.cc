// Copyright (c) 2017 ASMlover. All rights reserved.
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
#include <algorithm>
#include <vector>

class Component {
public:
  Component(void) = default;
  virtual ~Component(void) {}
  virtual void add_child(Component* /*child*/) {}
  virtual void del_child(Component* /*child*/) {}
  virtual Component* get_child(int /*index*/) { return nullptr; }
};

class Composite : public Component {
  std::vector<Component*> components_;
public:
  Composite(void) = default;
  virtual ~Composite(void) {
    for (auto* comp : components_)
      delete comp;
    components_.clear();
  }

  virtual void add_child(Component* child) override {
    components_.push_back(child);
  }

  virtual void del_child(Component* child) override {
    components_.erase(
        std::find(components_.begin(), components_.end(), child));
  }

  virtual Component* get_child(int index) override {
    if (index < 0 || static_cast<std::size_t>(index) >= components_.size())
      return nullptr;

    return components_[index];
  }
};

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  return 0;
}
