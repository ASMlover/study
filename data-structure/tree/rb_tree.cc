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
#include <tuple>
#include "common.hh"

namespace rb {

struct RBNodeBase;
using ColorType     = bool;
using BasePtr       = RBNodeBase*;
using ConstBasePtr  = const RBNodeBase*;

static constexpr ColorType kColorRed = false;
static constexpr ColorType kColorBlack = true;

struct RBNodeBase {
  ColorType color;
  BasePtr parent;
  BasePtr left;
  BasePtr right;

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

inline void __rbnode_replace(
    BasePtr x, BasePtr y, BasePtr p, BasePtr& root) noexcept {
  if (root == x)
    root = y;
  else if (p->left == x)
    p->left = y;
  else
    p->right = y;
}

inline void rbnode_rotate_left(BasePtr x, BasePtr& root) noexcept {
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
  __rbnode_replace(x, y, x->parent, root);
  y->left = x;
  x->parent = y;
}

inline void rbnode_rotate_right(BasePtr x, BasePtr& root) noexcept {
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
  __rbnode_replace(x, y, x->parent, root);
  y->right = x;
  x->parent = y;
}

inline void rbnode_insert_fix(BasePtr x, BasePtr& root) noexcept {
  while (x != root && x->parent->color == kColorRed) {
    if (x->parent->parent->left == x->parent) {
      //                |
      //               xpp
      //               / \
      //              p  [y]
      //             / \
      //           [x] [x]

      BasePtr y = x->parent->parent->right;
      if (y != nullptr && y->color == kColorRed) {
        x->parent->color = y->color = kColorBlack;
        x->parent->parent->color = kColorRed;
        x = x->parent->parent;
      }
      else {
        if (x->parent->right == x) {
          x = x->parent;
          rbnode_rotate_left(x, root);
        }
        x->parent->color = kColorBlack;
        x->parent->parent->color = kColorRed;
        rbnode_rotate_right(x->parent->parent, root);
      }
    }
    else {
      //                |
      //               xpp
      //               / \
      //             [y]  p
      //                 / \
      //               [x] [x]

      BasePtr y = x->parent->parent->left;
      if (y != nullptr && y->color == kColorRed) {
        x->parent->color = y->color = kColorBlack;
        x->parent->parent->color = kColorRed;
        x = x->parent->parent;
      }
      else {
        if (x->parent->left == x) {
          x = x->parent;
          rbnode_rotate_right(x, root);
        }
        x->parent->color = kColorBlack;
        x->parent->parent->color = kColorRed;
        rbnode_rotate_left(x->parent->parent, root);
      }
    }
  }
  root->color = kColorBlack;
}

inline void rbnode_insert(
    bool insert_left, BasePtr x, BasePtr p, RBNodeBase& header) noexcept {
  BasePtr& root = header.parent;

  x->parent = p;
  x->left = x->right = nullptr;
  x->color = kColorRed;

  if (insert_left) {
    p->left = x;
    if (p == &header)
      header.parent = header.right = x;
    else if (p == header.left)
      header.left = x;
  }
  else {
    p->right = x;
    if (p == header.right)
      header.right = x;
  }
  rbnode_insert_fix(x, root);
}

inline void rbnode_erase_fix(BasePtr x, BasePtr& root) noexcept {
  BasePtr w;
  while (x != root && x->color == kColorRed) {
    if (x == x->parent->left) {
      w = x->parent->right;
      if (w->color == kColorRed) {
        w->color = kColorBlack;
        x->parent->color = kColorRed;
        rbnode_rotate_left(x->parent, root);
        w = x->parent->right;
      }

      if (w->left->color == kColorBlack && w->right->color == kColorBlack) {
        w->color = kColorRed;
        x = x->parent;
      }
      else {
        if (w->right->color == kColorBlack) {
          w->left->color = kColorBlack;
          w->color = kColorRed;
          rbnode_rotate_right(w, root);
          w = x->parent->right;
        }
        w->color = x->parent->color;
        x->parent->color = kColorBlack;
        w->right->color = kColorBlack;
        rbnode_rotate_left(x->parent, root);
        x = root;
        break;
      }
    }
    else {
      w = x->parent->left;
      if (w->color == kColorRed) {
        w->color = kColorBlack;
        x->parent->color = kColorRed;
        rbnode_rotate_right(x->parent, root);
        w = w->parent->left;
      }
      if (w->left->color == kColorBlack && w->right->color == kColorBlack) {
        w->color = kColorRed;
        x = x->parent;
      }
      else {
        if (w->left->color == kColorBlack) {
          w->right->color = kColorBlack;
          w->color = kColorRed;
          rbnode_rotate_left(w, root);
          w = w->parent->left;
        }
        w->color = x->parent->color;
        x->parent->color = kColorBlack;
        w->left->color = kColorBlack;
        rbnode_rotate_right(x->parent, root);
        x = root;
        break;
      }
    }
  }

  if (x != nullptr)
    x->color = kColorBlack;
}

template <typename Value> struct RBNode : public RBNodeBase {
  Value value;
};

struct RBIterBase {
  BasePtr _node{};

  RBIterBase() noexcept {}
  RBIterBase(BasePtr x) noexcept : _node(x) {}
  RBIterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

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
    if (_node->color == kColorRed && _node->parent->parent == _node) {
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
struct RBIter : public RBIterBase {
  using Iter      = RBIter<_Tp, _Tp&, _Tp*>;
  using Self      = RBIter<_Tp, _Ref, _Ptr>;
  using Ref       = _Ref;
  using Ptr       = _Ptr;
  using Link      = RBNode<_Tp>*;
  using ConstLink = const RBNode<_Tp>*;

  RBIter() noexcept {}
  RBIter(BasePtr x) noexcept : RBIterBase(x) {}
  RBIter(ConstBasePtr x) noexcept : RBIterBase(x) {}
  RBIter(const Iter& x) noexcept : RBIterBase(x._node) {}

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

template <typename Tp> class RBTree final : private UnCopyable {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = RBIter<Tp, Tp&, Tp*>;
  using ConstIter = RBIter<Tp, const Tp&, const Tp*>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
private:
  using Node      = RBNode<Tp>;
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = Xt::SimpleAlloc<Node>;

  SizeType size_{};
  Node head_{};

  inline Link& root() noexcept { return (Link&)head_.parent; }
  inline ConstLink& root() const noexcept { return (ConstLink&)head_.parent; }
  inline Link& lmost() noexcept { return (Link&)head_.left; }
  inline ConstLink& lmost() const noexcept { return (ConstLink&)head_.left; }
  inline Link& rmost() noexcept { return (Link&)head_.right; }
  inline ConstLink& rmost() const noexcept { return (ConstLink&)head_.right; }
  inline Link _tail() noexcept { return &head_; }
  inline ConstLink _tail() const noexcept { return &head_; }

  static inline Link _parent(BasePtr x) noexcept { return Link(x->parent); }
  static inline ConstLink _parent(ConstBasePtr x) noexcept { return _parent(const_cast<BasePtr>(x)); }
  static inline Link _left(BasePtr x) noexcept { return Link(x->left); }
  static inline ConstLink _left(ConstBasePtr x) noexcept { return _left(const_cast<BasePtr>(x)); }
  static inline Link _right(BasePtr x) noexcept { return Link(x->right); }
  static inline ConstLink _right(ConstBasePtr x) noexcept { return _right(const_cast<BasePtr>(x)); }

  void _tear_node(Link x) {
    while (x != nullptr) {
      _tear_node(_right(x));
      Link y = _left(x);
      destroy_node(x);
      x = y;
    }
  }

  inline void init() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.color = kColorRed;
  }

  inline Link get_node() noexcept { return Alloc::allocate(); }
  inline void put_node(Link p) noexcept { Alloc::deallocate(p); }

  inline Link create_node(const ValueType& value) {
    Link tmp = get_node();
    try {
      Xt::construct(&tmp->value, value);
    }
    catch (...) {
      put_node(tmp);
      throw;
    }
    return tmp;
  }

  inline void destroy_node(Link p) {
    Xt::destroy(&p->value);
    put_node(p);
  }

  inline std::tuple<bool, Link, bool> find_insert_pos(const ValueType& value) {
    Link x = root();
    Link p = &head_;
    while (x != nullptr) {
      if (value == x->value)
        return std::make_tuple(false, nullptr, false);

      p = x;
      x = value < x->value ? _left(x) : _right(x);
    }
    bool insert_left = x != nullptr || p == &head_ || value < p->value;

    return std::make_tuple(true, p, insert_left);
  }

  inline void insert_aux(const ValueType& value) {
    auto [r, p, insert_left] = find_insert_pos(value);
    if (r) {
      rbnode_insert(insert_left, create_node(value), p, head_);
      ++size_;
    }
  }

  inline void erase_aux(Link p) {
  }
public:
  RBTree() noexcept { init(); }
  ~RBTree() noexcept { clear(); }

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

  template <typename Function> inline void for_each(Function&& fn) {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }

  void clear() {
    _tear_node(root());
    init();
  }

  void insert(const ValueType& x) { insert_aux(x); }
  void erase(ConstIter pos) {}
};

}

void test_rb() {
  rb::RBTree<int> t;
  auto show_rb = [&t] {
    std::cout << "\nrb-tree size: " << t.size() << std::endl;
    if (!t.empty())
      std::cout << "rb-tree: " << t.get_head() << ", " << t.get_tail() << std::endl;

    for (auto i = t.begin(); i != t.end(); ++i)
      std::cout << "rb-tree item value: " << *i << std::endl;
  };

  t.insert(45);
  t.insert(34);
  t.insert(78);
  t.insert(9);
  t.insert(23);
  t.insert(-56);
  show_rb();

  t.clear();
  show_rb();
}
