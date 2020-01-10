// Copyright (c) 2020 ASMlover. All rights reserved.
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
#include "xt_tree.hh"

namespace xt::tree {

namespace impl {
  inline BasePtr minimum(BasePtr x) noexcept {
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  inline ConstBasePtr minimum(ConstBasePtr x) noexcept {
    return minimum(const_cast<BasePtr>(x));
  }

  inline BasePtr maximum(BasePtr x) noexcept {
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  inline ConstBasePtr maximum(ConstBasePtr x) noexcept {
    return maximum(const_cast<BasePtr>(x));
  }
}

}

#include <iostream>

template <typename Tree> void show_t(Tree& t, const char* s = "tree") {
  std::cout << "\n" << s << " -> size: " << t.size() << std::endl;
  if (!t.empty()) {
    std::cout << s << " -> " << t.get_head() << "|" << t.get_tail() << std::endl;
  }
  for (auto i = t.begin(); i != t.end(); ++i)
    std::cout << s << " -> item value: " << *i << std::endl;
}

void test_tree2() {
  {
    xt::tree::AVLTree<int> t;
    show_t(t, "AVLTree");
  }

  {
    xt::tree::RBTree<int> t;
    show_t(t, "RBTree");
  }
}
