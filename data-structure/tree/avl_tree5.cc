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
#include <algorithm>
#include <iostream>
#include <utility>
#include "common.hh"

namespace avl5 {

template <typename Tp> inline void construct(Tp* p) noexcept {
  new ((void*)p) Tp();
}

template <typename T1, typename T2>
inline void construct(T1* p, const T2& x) noexcept {
  new ((void*)p) T1(x);
}

template <typename T1, typename T2>
inline void construct(T1* p, T2&& x) noexcept {
  new ((void*)p) T1(std::forward<T2>(x));
}

template <typename Tp, typename... Args>
inline void construct(Tp* p, Args&&... args) noexcept {
  new ((void*)p) Tp(std::forward<Args>(args)...);
}

template <typename Tp> inline void destroy(Tp* p) noexcept {
  p->~Tp();
}

template <typename Tp> class AllocWrapper final : private UnCopyable {
public:
  static Tp* allocate() noexcept {
    return (Tp*)std::malloc(sizeof(Tp));
  }

  static void deallocate(Tp* p) noexcept {
    std::free(p);
  }
};

struct AVLNodeBase;
using NodeBase      = AVLNodeBase;
using BasePtr       = NodeBase*;
using ConstBasePtr  = const NodeBase*;

struct AVLNodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;
  int height;

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

  inline int left_height() const noexcept {
    return left != nullptr ? left->height : 0;
  }

  inline int right_height() const noexcept {
    return right != nullptr ? right->height : 0;
  }

  inline void update_height() noexcept {
    height = std::max(left_height(), right_height()) + 1;
  }
};

inline BasePtr avlnode_next(BasePtr node) noexcept {
  if (node == nullptr)
    return nullptr;

  if (node->right != nullptr) {
    node = node->right;
    while (node->left != nullptr)
      node = node->left;
  }
  else {
    while (true) {
      AVLNodeBase* last = node;
      node = node->parent;
      if (node == nullptr || node == last)
        break;
    }
  }
  return node;
}

inline ConstBasePtr avlnode_next(ConstBasePtr node) noexcept {
  return avlnode_next(const_cast<BasePtr>(node));
}

template <typename Tp> struct AVLNode : public AVLNodeBase {
  using Node = AVLNode<Tp>;

  Tp value;

  static Node* from(BasePtr x) noexcept {
    return static_cast<Node*>(x);
  }

  static const Node* from(ConstBasePtr x) noexcept {
    return static_cast<const Node*>(x);
  }
};

template <typename _Tp, typename _Ref, typename _Ptr> struct AVLIter {
  using Iter  = AVLIter<_Tp, _Tp&, _Tp*>;
  using Self  = AVLIter<_Tp, _Ref, _Ptr>;
  using Ref   = _Ref;
  using Ptr   = _Ptr;
  using Node  = AVLNode<_Tp>;

  Node* _node{};

  AVLIter() noexcept {}
  AVLIter(BasePtr x) noexcept : _node(Node::from(x)) {}
  AVLIter(ConstBasePtr x) noexcept : _node(Node::from(x)) {}
  AVLIter(Node* x) noexcept : _node(x) {}
  AVLIter(const Node* x) noexcept : _node(const_cast<Node*>(x)) {}
  AVLIter(const Iter& x) noexcept : _node(x._node) {}

  inline bool operator==(const Self& x) const noexcept {
    return _node == x._node;
  }

  inline bool operator!=(const Self& x) const noexcept {
    return _node != x._node;
  }

  inline Ref operator*() const noexcept { return _node->value; }
  inline Ptr operator->() const noexcept { return &_node->value; }
};

template <typename Tp> class AVLTree final : private UnCopyable {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = AVLIter<Tp, Tp&, Tp*>;
  using ConstIter = AVLIter<Tp, const Tp&, const Tp*>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
private:
  using Node      = AVLNode<ValueType>;
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = AllocWrapper<Node>;

  SizeType size_{};
  Node head_{};

  static Link _parent(Link x) noexcept { return static_cast<Link>(x->parent); }
  static ConstLink _parent(ConstLink x) noexcept { return static_cast<ConstLink>(x->parent); }
  static Link _left(Link x) noexcept { return static_cast<Link>(x->left); }
  static ConstLink _left(ConstLink x) noexcept { return static_cast<ConstLink>(x->left); }
  static Link _right(Link x) noexcept { return static_cast<Link>(x->right); }
  static ConstLink _right(ConstLink x) noexcept { return static_cast<ConstLink>(x->right); }

  inline Link _root_link() noexcept { return static_cast<Link>(head_.parent); }
  inline ConstLink _root_link() const noexcept { return static_cast<ConstLink>(head_.parent); }
  inline Link _tail_link() noexcept { return static_cast<Link>(&head_); }
  inline ConstLink _tail_link() const noexcept { return static_cast<ConstLink>(&head_); }
};

}

void test_avl5() {
}
