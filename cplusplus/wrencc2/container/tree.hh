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
#pragma once

#include <algorithm>
#include <tuple>
#include "helper.hh"

namespace wrencc::tree {

using ColorType = bool;
static constexpr ColorType kColorRed = false;
static constexpr ColorType kColorBlk = true;
static constexpr int kHeightMark = 0xff;

struct NodeBase;
struct AVLNodeBase;
struct RBNodeBase;
using BasePtr       = NodeBase*;
using ConstBasePtr  = const NodeBase*;

namespace impl::avl {
  void insert(bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept;
  void erase(BasePtr x, NodeBase& root) noexcept;
}

namespace impl::rb {
  void insert(bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept;
  void erase(BasePtr x, NodeBase& header) noexcept;
}

struct NodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;

  template <typename AVL = AVLNodeBase> inline AVL* as_avl() noexcept {
    return static_cast<AVL*>(this);
  }

  template <typename RB = AVLNodeBase> inline RB* as_rb() noexcept {
    return static_cast<RB*>(this);
  }

  static BasePtr successor(BasePtr x) noexcept {
    if (x->right != nullptr) {
      x = x->right;
      while (x->left != nullptr)
        x = x->left;
    }
    else {
      BasePtr y = x->parent;
      while (x == y->right) {
        x = y;
        y = y->parent;
      }
      if (x->right != y)
        x = y;
    }
    return x;
  }

  template <typename HeadChecker>
  static BasePtr predecessor(BasePtr x, HeadChecker&& checker) noexcept {
    if (checker(x) && x->parent->parent == x) {
      x = x->right;
    }
    else if (x->left != nullptr) {
      x = x->left;
      while (x->right != nullptr)
        x = x->right;
    }
    else {
      BasePtr y = x->parent;
      while (x == y->left) {
        x = y;
        y = y->parent;
      }
      x = y;
    }
    return x;
  }
};

struct AVLNodeBase : public NodeBase {
  int height;

  inline void set_marker() noexcept { height = kHeightMark; }
  inline bool is_marker() const noexcept { return height == kHeightMark; }
  inline void set_height(int h) noexcept { height = h; }

  inline int lheight() const noexcept {
    return left != nullptr ? left->as_avl()->height : 0;
  }

  inline int rheight() const noexcept {
    return right != nullptr ? right->as_avl()->height : 0;
  }

  inline void update_height() noexcept {
    height = std::max(lheight(), rheight()) + 1;
  }
};

struct RBNodeBase : public NodeBase {
  ColorType color;

  inline void set_marker() noexcept { as_red(); }
  inline bool is_marker() const noexcept { return is_red(); }

  inline bool is_red() const noexcept { return color == kColorRed; }
  inline bool is_blk() const noexcept { return color == kColorBlk; }
  inline void as_red() noexcept { color = kColorRed; }
  inline void as_blk() noexcept { color = kColorBlk; }
  inline void set_color(ColorType c) noexcept { color = c; }
};

template <typename Value> struct AVLNode : public AVLNodeBase {
  Value value;
};

template <typename Value> struct RBNode : public RBNodeBase {
  Value value;
};

template <typename _Tp, typename _Ref, typename _Ptr, typename _Node>
struct TreeIter {
  using Iter = TreeIter<_Tp, _Tp&, _Tp*, _Node>;
  using Self = TreeIter<_Tp, _Ref, _Ptr, _Node>;
  using Ref  = _Ref;
  using Ptr  = _Ptr;
  using Link = _Node*;

  BasePtr _node{};

  TreeIter() noexcept {}
  TreeIter(BasePtr x) noexcept : _node(x) {}
  TreeIter(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}
  TreeIter(const Iter& x) noexcept : _node(x._node) {}

  inline void increment() noexcept { _node = NodeBase::successor(_node); }

  template <typename HeadChecker>
  inline void decrement(HeadChecker&& checker) noexcept {
    _node = NodeBase::predecessor(_node, std::move(checker));
  }

  inline Link node() const noexcept { return Link(_node); }
  inline Ref operator*() const noexcept { return Link(_node)->value; }
  inline Ptr operator->() const noexcept { return &Link(_node)->value; }

  inline bool operator==(const Self& r) const noexcept {
    return _node == r._node;
  }

  inline bool operator!=(const Self& r) const noexcept {
    return _node != r._node;
  }

  Self& operator++() noexcept { increment(); return *this; }
  Self operator++(int) noexcept { Self tmp(*this); increment(); return tmp; }

  Self& operator--() noexcept {
    decrement([](BasePtr x) { return Link(x)->is_marker(); });
    return *this;
  }

  Self operator--(int) noexcept {
    Self tmp(*this);
    decrement([](BasePtr x) { return Link(x)->is_marker(); });
    return tmp;
  }
};

template <typename Tp,
          typename Node,
          typename Less = std::less<Tp>,
          typename Equal = std::equal_to<Tp>>
class TreeBase : private UnCopyable {
public:
  using ValueType = Tp;
  using Iter      = TreeIter<Tp, Tp&, Tp*, Node>;
  using ConstIter = TreeIter<Tp, const Tp&, const Tp*, Node>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
protected:
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = wrencc::SimpleAlloc<Node>;

  sz_t size_{};
  Node head_{};
  Less lt_comp_{};
  Equal eq_comp_{};

  static inline Link _parent(BasePtr x) noexcept { return Link(x)->parent; }
  static inline ConstLink _parent(ConstBasePtr x) noexcept { return ConstLink(x)->parent; }
  static inline Link _left(BasePtr x) noexcept { return Link(x)->left; }
  static inline ConstLink _left(ConstBasePtr x) noexcept { return ConstLink(x)->left; }
  static inline Link _right(BasePtr x) noexcept { return Link(x)->right; }
  static inline ConstLink _right(ConstBasePtr x) noexcept { return ConstLink(x)->right; }

  inline void init() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.set_marker();
  }

  inline Link root() noexcept { return Link(head_.parent); }
  inline ConstLink root() const noexcept { return ConstLink(head_.parent); }
  inline Link tail() noexcept { return Link(&head_); }
  inline ConstLink tail() const noexcept { return ConstLink(&head_); }
  inline Link lmost() noexcept { return Link(head_.left); }
  inline ConstLink lmost() noexcept { return ConstLink(head_.left); }
  inline Link rmost() noexcept { return Link(head_.right); }
  inline ConstLink rmost() const noexcept { return ConstLink(head_.right); }

  inline Link get_node() noexcept { return Alloc::allocate(); }
  inline void put_node(Link p) noexcept { Alloc::deallocate(p); }

  Link create_node(const ValueType& val) {
    Link tmp = get_node();
    try {
      construct(&tmp->value, val);
    }
    catch (...) {
      put_node(tmp);
      throw;
    }
    return tmp;
  }

  Link create_node(ValueType&& val) {
    Link tmp = get_node();
    try {
      construct(&tmp->value, std::move(val));
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
      construct(&tmp->value, std::forward<Args>(args)...);
    }
    catch (...) {
      put_node(tmp);
      throw;
    }
    return tmp;
  }

  void destroy_node(Link p) {
    destroy(&p->value);
    put_node(p);
  }

  inline std::tuple<bool, Link, bool> find_insertion_pos(const ValueType& key) {
    Link x = root();
    Link p = tail();
    while (x != nullptr) {
      if (eq_comp_(key, x->value))
        return std::make_tuple(false, nullptr, false);

      p = x;
      x = lt_comp_(key, x->value) ? _left(x) : _right(x);
    }
    bool insert_left = x != nullptr || p == tail() || lt_comp_(key, p->value);

    return std::make_tuple(true, p, insert_left);
  }

  template <typename Insertion>
  inline void insert_aux(Insertion&& insert_fn, const ValueType& value) {
    auto [r, p, insert_left] = find_insertion_pos(value);
    if (r) {
      insert_fn(insert_left, create_node(value), p, head_);
      ++size_;
    }
  }

  template <typename Insertion>
  inline void insert_aux(Insertion&& insert_fn, ValueType&& value) {
    auto [r, p, insert_left] = find_insertion_pos(value);
    if (r) {
      insert_fn(insert_left, create_node(std::move(value)), p, head_);
      ++size_;
    }
  }

  template <typename Insertion, typename... Args>
  inline void insert_aux(Insertion&& insert_fn, Args&&... args) {
    Link tmp = create_node(std::forward<Args>(args)...);
    auto [r, p, insert_left] = find_insertion_pos(tmp->value);
    if (r) {
      insert_fn(insert_left, tmp, p, head_);
      ++size_;
    }
    else {
      destroy_node(tmp);
    }
  }

  template <typename Eraser> inline void erase_aux(Eraser&& erase_fn, Link p) {
    if (size_ != 0) {
      erase_fn(p, head_);
      destroy_node(p);
      --size_;
    }
  }

  void erase_subtree(Link x) {
    while (x != nullptr) {
      erase_subtree(_right(x));
      Link y = _left(x);
      destroy_node(x);
      x = y;
    }
  }

  inline ConstLink find_aux(const ValueType& key) const noexcept {
    ConstLink x = root();
    ConstLink y = tail();
    while (x != nullptr) {
      if (eq_comp_(key, x->value)) {
        y = x;
        break;
      }
      x = lt_comp_(key, x->value) ? _left(x) : _right(x);
    }
    return y;
  }
public:
  TreeBase() noexcept { init(); }
  ~TreeBase() noexcept { clear(); }

  inline bool empty() const noexcept { return size_ == 0; }
  inline sz_t size() const noexcept { return size_; }
  inline Iter begin() noexcept { return head_.left; }
  inline ConstIter begin() const noexcept { return head_.left; }
  inline Iter end() noexcept { return &head_; }
  inline ConstIter end() const noexcept { return &head_; }
  inline Ref get_head() noexcept { return *begin(); }
  inline ConstRef get_head() const noexcept { return *begin(); }
  inline Ref get_tail() noexcept { return *(--end()); }
  inline ConstRef get_tail() const noexcept { return *(--end()); }

  inline void clear() {
    erase_subtree(root());
    init();
  }

  inline Iter find(const ValueType& key) noexcept {
    return find_aux(key);
  }

  inline ConstIter find(const ValueType& key) const noexcept {
    return find_aux(key);
  }

  template <typename Visitor> inline void for_each(Visitor&& visitor) {
    for (auto i = begin(); i != end(); ++i)
      visitor(*i);
  }
};

}

namespace wrencc {

template <typename Tp>
class AVLTree final : public tree::TreeBase<Tp, tree::AVLNode<Tp>> {
};

template <typename Tp>
class RBTree final : public tree::TreeBase<Tp, tree::RBNode<Tp>> {
};

}
