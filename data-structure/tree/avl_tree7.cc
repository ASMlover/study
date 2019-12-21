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
#include <tuple>
#include "common.hh"

namespace avl7 {

struct AVLNodeBase;
using BasePtr       = AVLNodeBase*;
using ConstBasePtr  = const AVLNodeBase*;

static constexpr int kHeightMask = 0xff;

struct AVLNodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;
  int height;

  inline int lheight() const noexcept { return left ? left->height : 0; }
  inline int rheight() const noexcept { return right ? right->height : 0; }
  inline void update_height() noexcept { height = Xt::max(lheight(), rheight()) + 1; }

  static inline BasePtr _minimum(BasePtr x) noexcept {
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  static inline ConstBasePtr _minimum(ConstBasePtr x) noexcept {
    return _minimum(const_cast<BasePtr>(x));
  }

  static inline BasePtr _maximum(BasePtr x) noexcept {
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  static inline ConstBasePtr _maximum(ConstBasePtr x) noexcept {
    return _maximum(const_cast<BasePtr>(x));
  }
};

inline void __avlnode_replace_child(
    BasePtr x, BasePtr y, BasePtr p, BasePtr& root) noexcept {
  if (p != nullptr) {
    if (root == x)
      root = y;
    else if (p->left == x)
      p->left = y;
    else
      p->right = y;
  }
  else {
    root = y;
  }
}

inline BasePtr avlnode_rotate_left(BasePtr x, BasePtr& root) noexcept {
  //          |                   |
  //          x                   y
  //           \                 / \
  //            y               x   b
  //           / \               \
  //         [a]  b              [a]

  BasePtr y = x->right;
  x->right = y->left;
  if (x->right != nullptr)
    x->right->parent = x;
  y->parent = x->parent;
  __avlnode_replace_child(x, y, x->parent, root);
  y->left = x;
  x->parent = y;
  x->update_height();
  y->update_height();

  return y;
}

inline BasePtr avlnode_rotate_right(BasePtr x, BasePtr& root) noexcept {
  //          |                   |
  //          x                   y
  //         /                   / \
  //        y                   a   x
  //       / \                     /
  //      a  [b]                 [b]

  BasePtr y = x->left;
  x->left = y->right;
  if (x->left != nullptr)
    x->left->parent = x;
  y->parent = x->parent;
  __avlnode_replace_child(x, y, x->parent, root);
  y->right = x;
  x->parent = y;
  x->update_height();
  y->update_height();

  return y;
}

template <typename ValueType> struct AVLNode : public AVLNodeBase {
  ValueType value;
};

struct AVLIterBase {
  BasePtr _node{};

  AVLIterBase() noexcept {}
  AVLIterBase(BasePtr x) noexcept : _node(x) {}
  AVLIterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

  void increment() noexcept {
    if (_node->right != nullptr) {
      _node = _node->right;
      while (_node->left != nullptr)
        _node = _node->left;
    }
    else {
      BasePtr y = _node->parent;
      while (_node == y->right) {
        _node = y;
        y = y->parent;
      }
      if (_node->right != y)
        _node = y;
    }
  }

  void decrement() noexcept {
    if (_node->height == kHeightMask && _node->parent->parent == _node) {
      _node = _node->right;
    }
    else if (_node->left != nullptr) {
      _node = _node->left;
      while (_node->right != nullptr)
        _node = _node->right;
    }
    else {
      BasePtr y = _node->parent;
      while (_node == y->left) {
        _node = y;
        y = y->parent;
      }
      _node = y;
    }
  }
};

template <typename _Tp, typename _Ref, typename _Ptr>
struct AVLIter : public AVLIterBase {
  using Iter      = AVLIter<_Tp, _Tp&, _Tp*>;
  using Self      = AVLIter<_Tp, _Ref, _Ptr>;
  using Ref       = _Ref;
  using Ptr       = _Ptr;
  using Link      = AVLNode<_Tp>*;
  using ConstLink = const AVLNode<_Tp>*;

  AVLIter() noexcept {}
  AVLIter(BasePtr x) noexcept : AVLIterBase(x) {}
  AVLIter(ConstBasePtr x) noexcept : AVLIterBase(x) {}
  AVLIter(const Iter& x) noexcept : AVLIterBase(x._node) {}

  inline Link node() noexcept { return Link(_node); }
  inline ConstLink node() const noexcept { return ConstLink(_node); }

  inline bool operator==(const Self& r) const noexcept { return _node == r._node; }
  inline bool operator!=(const Self& r) const noexcept { return _node != r._node; }
  inline Ref operator*() const noexcept { return Link(_node)->value; }
  inline Ptr operator->() const noexcept { return &Link(_node)->value; }
  Self& operator++() noexcept { increment(); return *this; }
  Self operator++(int) noexcept { Self tmp(*this); increment(); return tmp; }
  Self& operator--() noexcept { decrement(); return *this; }
  Self operator--(int) noexcept { Self tmp(*this); decrement(); return tmp; }
};

template <typename Tp,
         typename Less = std::less<Tp>, typename Equal = std::equal_to<Tp>>
class AVLTree final : private UnCopyable {
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
  using Alloc     = Xt::SimpleAlloc<Node>;

  SizeType size_{};
  Node head_{};
  Less less_comp_{};
  Equal equal_comp_{};

  inline Link& _root() noexcept { return (Link&)head_.parent; }
  inline ConstLink& _root() const noexcept { return (ConstLink&)head_.parent; }
  inline Link& _tail() noexcept { return (Link&)&head_; }
  inline ConstLink& _tail() const noexcept { return (ConstLink&)&head_; }
  inline Link& _lmost() noexcept { return (Link&)head_.left; }
  inline ConstLink& _lmost() const noexcept { return (ConstLink&)head_.left; }
  inline Link& _rmost() noexcept { return (Link&)head_.right; }
  inline ConstLink& _rmost() const noexcept { return (ConstLink&)head_.right; }

  static inline Link _parent(BasePtr& x) noexcept { return Link(x->parent); }
  static inline ConstLink _parent(ConstBasePtr x) noexcept { return _parent(const_cast<BasePtr>(x)); }
  static inline Link _left(BasePtr x) noexcept { return Link(x->left); }
  static inline ConstLink _left(ConstBasePtr x) noexcept { return _left(const_cast<BasePtr>(x)); }
  static inline Link _right(BasePtr x) noexcept { return Link(x->right); }
  static inline ConstLink _right(ConstBasePtr x) noexcept { return _right(const_cast<BasePtr>(x)); }

  inline Link get_node(const ValueType& x) { return Alloc::allocate(); }
  inline void put_node(Link p) { Alloc::deallocate(p); }

  inline void init() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.height = kHeightMask;
  }

  Link create_node(const ValueType& x) {
    Link tmp = get_node(x);
    try {
      Xt::construct(&tmp->value, x);
    }
    catch (...) {
      put_node(tmp);
      throw;
    }
    return tmp;
  }

  void destroy_node(Link p) {
    Xt::destroy(&p->value);
    put_node(p);
  }
public:
  AVLTree() noexcept { init(); }
  ~AVLTree() noexcept {}
};


}

void test_avl7() {
  avl7::AVLTree<int> t;
}
