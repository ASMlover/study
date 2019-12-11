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
#include <algorithm>
#include <utility>
#include "common.hh"

namespace avl4 {

template <typename Tp> inline void construct(Tp* p) noexcept {
  new ((void*)p) Tp();
}

template <typename Tp1, typename Tp2>
inline void construct(Tp1* p, const Tp2& v) noexcept {
  new ((void*)p) Tp2(v);
}

template <typename Tp1, typename Tp2>
inline void construct(Tp1* p, Tp2&& v) noexcept {
  new ((void*)p) Tp2(std::forward<Tp2>(v));
}

template <typename Tp, typename... Args>
inline void construct(Tp* p, Args&&... args) noexcept {
  new ((void*)p) Tp(std::forward<Args>(args)...);
}

template <typename Tp> inline void destroy(Tp* p) noexcept {
  p->~Tp();
}

template <typename Tp> class AVLAlloc final : private UnCopyable {
public:
  static Tp* allocate() noexcept {
    return (Tp*)std::malloc(sizeof(Tp));
  }

  static void deallocate(Tp* p) noexcept {
    std::free(p);
  }
};

struct AVLNodeBase {
  using BasePtr       = AVLNodeBase*;
  using ConstBasePtr  = const AVLNodeBase*;

  BasePtr parent;
  BasePtr left;
  BasePtr right;
  int bal_factor;

  inline BasePtr get_minimum() noexcept {
    BasePtr x = this;
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  inline ConstBasePtr get_minimum() const noexcept {
    ConstBasePtr x = this;
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  inline BasePtr get_maximum() noexcept {
    BasePtr x = this;
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  inline ConstBasePtr get_maximum() const noexcept {
    ConstBasePtr x = this;
    while (x->right != nullptr)
      x = x->right;
    return x;
  }
};

inline AVLNodeBase* avlnode_inc(AVLNodeBase* x) noexcept {
  if (x->right != nullptr) {
    x = x->right;
    while (x->left != nullptr)
      x = x->left;
  }
  else {
    AVLNodeBase* p = x->parent;
    while (x == p->right) {
      x = p;
      x = p->parent;
    }
    if (x->right != p)
      x = p;
  }
  return x;
}

inline AVLNodeBase* avlnode_dec(AVLNodeBase* x) noexcept {
  if (x->parent->parent == x && x->bal_factor == -2) {
    x = x->right;
  }
  else if (x->left != nullptr) {
    x = x->left;
    while (x->right != nullptr)
      x = x->right;
  }
  else {
    AVLNodeBase* p = x->parent;
    while (x == p->left) {
      x = p;
      p = p->parent;
    }
    x = p;
  }
  return x;
}

template <typename Tp> struct AVLNode : public AVLNodeBase {
  Tp value;
};

template <typename _Tp, typename _Ptr, typename _Ref> struct AVLIter {
  using Iter      = AVLIter<_Tp, _Tp*, _Tp&>;
  using Self      = AVLIter<_Tp, _Ptr, _Ref>;
  using ValueType = _Tp;
  using Ptr       = _Ptr;
  using Ref       = _Ref;
  using Node      = AVLNode<_Tp>;

  Node* node{};

  AVLIter() noexcept {}
  AVLIter(AVLNodeBase* x) noexcept : node(static_cast<Node*>(x)) {}
  AVLIter(Node* x) noexcept : node(x) {}
  AVLIter(const Node* x) noexcept : node(x) {}
  AVLIter(const Iter& x) noexcept : node(x.node) {}

  inline bool operator==(const Self& x) const noexcept { return node == x.node; }
  inline bool operator!=(const Self& x) const noexcept { return node != x.node; }

  inline Ref operator*() const noexcept { return node->value; }
  inline Ptr operator->() const noexcept { return &node->value; }

  inline Self& operator++() noexcept {
    node = static_cast<Node*>(avlnode_inc(node));
    return *this;
  }

  inline Self operator++(int) noexcept {
    Self tmp(*this);
    node = static_cast<Node*>(avlnode_inc(node));
    return tmp;
  }

  inline Self& operator--() noexcept {
    node = static_cast<Node*>(avlnode_dec(node));
    return *this;
  }

  inline Self operator--(int) noexcept {
    Self tmp(*this);
    node = static_cast<Node*>(avlnode_dec(node));
    return tmp;
  }
};

template <typename Tp> class AVLTree final : private UnCopyable {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = AVLIter<Tp, Tp*, Tp&>;
  using ConstIter = AVLIter<Tp, const Tp*, const Tp&>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
private:
  using Node      = AVLNode<ValueType>;
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = AVLAlloc<Node>;

  SizeType size_{};
  Node head_{};

  inline void initialize() noexcept {
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.bal_factor = -2;
  }

  inline Link _head_link() noexcept { return static_cast<Link>(head_.parent); }
  inline ConstLink _head_link() const noexcept { return static_cast<ConstLink>(head_.parent); }
  inline Link _tail_link() noexcept { return static_cast<Link>(&head_); }
  inline ConstLink _tail_link() const noexcept { return static_cast<ConstLink>(&head_); }

  static Link _parent(Link x) noexcept { return static_cast<Link>(x->parent); }
  static ConstLink _parent(ConstLink x) noexcept { return static_cast<ConstLink>(x->parent); }
  static Link _left(Link x) noexcept { return static_cast<Link>(x->left); }
  static ConstLink _left(ConstLink x) noexcept { return static_cast<ConstLink>(x->left); }
  static Link _right(Link x) noexcept { return static_cast<Link>(x->right); }
  static ConstLink _right(ConstLink x) noexcept { return static_cast<ConstLink>(x->right); }

public:
  AVLTree() noexcept {
    initialize();
  }

  ~AVLTree() noexcept {
    // TODO:
  }

  inline bool empty() const noexcept { return size_ == 0; }
  inline SizeType size() const noexcept { return size_; }
  inline Iter begin() noexcept { return Iter(head_.left); }
  inline ConstIter begin() const noexcept { return ConstIter(head_.left); }
  inline Iter end() noexcept { return Iter(&head_); }
  inline ConstIter end() const noexcept { return ConstIter(&head_); }
  inline Ref get_head() noexcept { return *begin(); }
  inline ConstRef get_head() const noexcept { return *begin(); }
  inline Ref get_tail() noexcept { return *(--end()); }
  inline ConstRef get_tail() const noexcept { return *(--end()); }

  template <typename Function> inline void for_each(Function&& fn) noexcept {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }
};

}
