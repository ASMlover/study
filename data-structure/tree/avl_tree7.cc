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

inline BasePtr avlnode_fix_left(BasePtr a, BasePtr& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //       [d]  b              [d]  c               /   \
  //           / \                 / \             a     b
  //          c  [g]              e   b           / \   / \
  //         / \                     / \        [d]  e f  [g]
  //        e   f                   f  [g]

  BasePtr b = a->right;
  if (b->lheight() > b->rheight())
    avlnode_rotate_right(b, root);
  a = avlnode_rotate_left(a, root);

  return a;
}

inline BasePtr avlnode_fix_right(BasePtr a, BasePtr& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //        b  [g]              c  [g]              /   \
  //       / \                 / \                 b     a
  //     [d]  c               b   f               / \   / \
  //         / \             / \                [d]  e f  [g]
  //        e   f          [d]  e

  BasePtr b = a->left;
  if (b->lheight() < b->rheight())
    avlnode_rotate_left(b, root);
  a = avlnode_rotate_right(a, root);

  return a;
}

inline void avlnode_rebalance(BasePtr node, BasePtr& root) noexcept {
  while (node != root) {
    int lh = node->lheight();
    int rh = node->rheight();
    int height = Xt::max(lh, rh) + 1;
    int diff = lh - rh;

    if (node->height != height)
      node->height = height;
    else if (diff >= -1 && diff <= 1)
      break;

    if (diff <= -2)
      node = avlnode_fix_left(node, root);
    else if (diff >= 2)
      node = avlnode_fix_right(node, root);

    node = node->parent;
  }
}

inline void avlnode_insert(
    bool insert_left, BasePtr x, BasePtr p, AVLNodeBase& header) noexcept {
  BasePtr& root = header.parent;

  x->parent = p;
  x->left = x->right = nullptr;
  x->height = 0;

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
  avlnode_rebalance(x, root);
}

inline void avlnode_erase(BasePtr x, AVLNodeBase& header) noexcept {
  BasePtr& root = header.parent;
  BasePtr& lmost = header.left;
  BasePtr& rmost = header.right;
  BasePtr orig = x;

  BasePtr p = nullptr;
  BasePtr y = nullptr;
  if (x->left != nullptr && x->right != nullptr) {
    x = x->right;
    while (x->left != nullptr)
      x = x->left;
    y = x->right;
    p = x->parent;
    if (y != nullptr)
      y->parent = p;
    __avlnode_replace_child(x, y, p, root);
    if (x->parent == orig)
      p = x;
    x->parent = orig->parent;
    x->left = orig->left;
    x->right = orig->right;
    x->height = orig->height;
    __avlnode_replace_child(orig, x, x->parent, root);
    if (x->right != nullptr)
      x->right->parent = x;
  }
  else {
    y = x->left != nullptr ? x->left : x->right;
    p = x->parent;
    __avlnode_replace_child(x, y, p, root);
    if (y != nullptr)
      y->parent = p;
  }
  if (p != nullptr)
    avlnode_rebalance(p, root);

  if (root == orig)
    root = y;

  if (lmost == orig || lmost == nullptr)
    lmost = root != nullptr ? AVLNodeBase::_minimum(root) : &header;
  if (rmost == orig || rmost == nullptr)
    rmost = root != nullptr ? AVLNodeBase::_maximum(root) : &header;
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

  inline Link get_node() { return Alloc::allocate(); }
  inline void put_node(Link p) { Alloc::deallocate(p); }

  inline void init() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.height = kHeightMask;
  }

  Link create_node(const ValueType& x) {
    Link tmp = get_node();
    try {
      Xt::construct(&tmp->value, x);
    }
    catch (...) {
      put_node(tmp);
      throw;
    }
    return tmp;
  }

  Link create_node(ValueType&& x) {
    Link tmp = get_node();
    try {
      Xt::construct(&tmp->value, std::move(x));
    }
    catch (...) {
      put_node(tmp);
      throw;
    }
    return tmp;
  }

  template <typename... Args> Link create_node(Args&&... args) {
    Link tmp = get_node();
    try {
      Xt::construct(&tmp->value, std::forward<Args>(args)...);
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

  inline std::tuple<bool, Link, bool> find_insert_pos(const ValueType& value) {
    Link x = _root();
    Link y = &head_;
    while (x != nullptr) {
      if (equal_comp_(value, x->value))
        return std::make_tuple(false, nullptr, false);

      y = x;
      x = less_comp_(value, x->value) ? _left(x) : _right(x);
    }

    bool insert_left = x != nullptr || y == &head_ || less_comp_(value, y->value);
    return std::make_tuple(true, y, insert_left);
  }

  void insert_aux(const ValueType& value) {
    auto [r, p, insert_left] = find_insert_pos(value);
    if (r) {
      avlnode_insert(insert_left, create_node(value), p, head_);
      ++size_;
    }
  }

  void insert_aux(ValueType&& value) {
    auto [r, p, insert_left] = find_insert_pos(value);
    if (r) {
      avlnode_insert(insert_left, create_node(std::move(value)), p, head_);
      ++size_;
    }
  }

  template <typename... Args> void insert_aux(Args&&... args) {
    Link node = create_node(std::forward<Args>(args)...);
    auto [r, p, insert_left] = find_insert_pos(node->value);
    if (r) {
      avlnode_insert(insert_left, node, p, head_);
      ++size_;
    }
    else {
      put_node(node);
    }
  }

  void erase_aux(Link p) {
    if (!empty()) {
      avlnode_erase(p, head_);
      destroy_node(p);
      --size_;
    }
  }

  inline ConstLink find_aux(const ValueType& key) const noexcept {
    ConstLink x = _root();
    ConstLink y = &head_;
    while (x != nullptr) {
      if (equal_comp_(key, x->value)) {
        y = x;
        break;
      }
      else {
        x = less_comp_(key, x->value) ? _left(x) : _right(x);
      }
    }
    return y;
  }
public:
  AVLTree() noexcept { init(); }
  ~AVLTree() noexcept {}

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

  void insert(const ValueType& x) { insert_aux(x); }
  void insert(ValueType&& x) { insert_aux(std::move(x)); }

  template <typename... Args> void insert(Args&&... args) {
    insert_aux(std::forward<Args>(args)...);
  }

  void erase(Iter pos) { erase_aux(pos.node()); }
  void erase(ConstIter pos) { erase_aux(pos.node()); }

  Iter find(const ValueType& key) noexcept {
    return Iter(find_aux(key));
  }

  ConstIter find(const ValueType& key) const noexcept {
    return ConstIter(find_aux(key));
  }
};


}

void test_avl7() {
  avl7::AVLTree<int> t;
  auto show_avl = [&t] {
    std::cout << "\navl-tree count is: " << t.size() << std::endl;
    if (!t.empty())
      std::cout << "avl-tree {" << t.get_head() << ", " << t.get_tail() << "}" << std::endl;
    t.for_each([](int value) {
        std::cout << "avl-tree item value is: " << value << std::endl;
        });
  };

  auto show_reverse = [&t] {
    for (auto i = t.end(); i != t.begin();)
      std::cout << "avl-tree reverse item value is: " << *(--i) << std::endl;
  };

  t.insert(34);
  t.insert(67);
  t.insert(56);
  t.insert(45);
  t.insert(23);
  t.insert(13);
  t.insert(3);
  t.insert(7);
  show_avl();
  show_reverse();

  t.erase(t.begin());
  show_avl();
  show_reverse();

  std::cout << "find 45: " << (t.find(45) != t.end()) << std::endl;
}
