// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <vector>
#include "utility.hh"
#include "lexer.hh"
#include "./container/array_list.hh"
#include "./container/linked_list.hh"
#include "./container/double_list.hh"
#include "./container/tree.hh"

struct Dummy {
  int _xval{};
  int _yval{};

  Dummy(int x, int y) noexcept : _xval(x), _yval(y) {}

  void show() noexcept {
    std::cout << "Dummy {" << _xval << ", " << _yval << "}" << std::endl;
  }
};

static void tree_test() {
  auto repr = [](auto& t, const char* s) {
    std::cout << "\n" << s << " size: " << t.size() << std::endl;
    if (!t.empty()) {
      std::cout
        << s << " {"
        << t.get_head() << ", " << t.get_tail()
        << "}" << std::endl;
    }
    for (auto i = t.begin(); i != t.end(); ++i)
      std::cout << s << " item value is: " << *i << std::endl;
  };

  {
    wrencc::AVLTree<int> t;
    repr(t, "AVLTree");

    t.insert(45);
    t.insert(23);
    t.insert(67);
    t.insert(78);
    t.insert(52);
    t.insert(49);
    t.insert(19);
    t.insert(2);
    t.insert(7);
    repr(t, "AVLTree");

    std::cout << "\nAVLTree find <49> " << (t.find(49) != t.end()) << std::endl;

    t.erase(t.begin());
    t.erase(--t.end());
    t.erase(t.find(49));
    repr(t, "AVLTree");
    std::cout << "\nAVLTree find <49> " << (t.find(49) != t.end()) << std::endl;

    t.clear();
    repr(t, "AVLTree");
  }

  {
    wrencc::RBTree<int> t;
    repr(t, "RBTree");

    t.insert(45);
    t.insert(23);
    t.insert(67);
    t.insert(78);
    t.insert(52);
    t.insert(49);
    t.insert(19);
    t.insert(2);
    t.insert(7);
    repr(t, "RBTree");

    std::cout << "\nRBTree find <49> " << (t.find(49) != t.end()) << std::endl;

    t.erase(t.begin());
    t.erase(--t.end());
    t.erase(t.find(49));
    repr(t, "RBTree");
    std::cout << "\nRBTree find <49> " << (t.find(49) != t.end()) << std::endl;

    t.clear();
    repr(t, "RBTree");
  }
}

int main(int argc, char* argv[]) {
  (void)argc, (void)argv;

  wrencc::SymbolTable st;
  st.append("Hello");
  st.ensure("Hello");
  std::cout << st.count() << " " << st.find("Hello") << std::endl;
  st.for_each([](const wrencc::String& s) {
      std::cout << "[item value] -> " << s << std::endl;
      });

  wrencc::Lexer lex("class A {}");
  do {
    auto t = lex.next_token();
    std::cout << t << std::endl;
    if (t.kind() == wrencc::TokenKind::TK_EOF)
      break;
  } while (true);

  wrencc::ArrayList<int> v(5, 5);
  v.append(45);
  v.insert(v.begin(), 66);
  v.insert(v.end(), 77);
  v.insert(v.begin() + 3, 88);
  v.for_each([](int v) {
        std::cout << "[item value] -> " << v << std::endl;
      });
  std::cout << v.size() << " -> " << v.capacity() << std::endl;
  std::cout << v.get_head() << " -> " << v.get_tail() << std::endl;

  for (auto x : v)
    std::cout << x << std::endl;

  v.resize(20);
  std::cout << v.size() << " -> " << v.capacity() << std::endl;
  std::cout << v.pop() << std::endl << std::endl;

  wrencc::ArrayList<int> vv(std::move(v));
  std::cout << v.size() << " -> " << v.capacity() << std::endl;

  wrencc::LinkedList<int> l;
  l.append(1);
  l.append(2);
  l.for_each([](int x) {
      std::cout << "[list item value] -> " << x << std::endl;
      });
  for (auto x : l)
    std::cout << "iterator linked-list value: " << x << std::endl;
  while (!l.empty()) {
    int x = l.pop_head();
    std::cout << "[list pop item value] -> " << x << std::endl;
  }
  std::cout << "list info: " << l.size() << std::endl;

  wrencc::LinkedList<Dummy> dummy_list;
  dummy_list.append(11, 22);
  dummy_list.append(111, 222);
  dummy_list.append(1111, 2222);
  for (auto& d : dummy_list)
    d.show();

  wrencc::DoubleList<int> dl;
  dl.append(11);
  dl.append(22);
  dl.for_each([](int x) {
      std::cout << "[double linked list item value] -> " << x << std::endl;
      });
  dl.pop_head();
  dl.for_each([](int x) {
      std::cout << "[double linked list item value] -> " << x << std::endl;
      });
  std::cout << "double linked list: " << dl.size() << std::endl;

  tree_test();

  return 0;
}
