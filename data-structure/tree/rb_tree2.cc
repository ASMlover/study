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

namespace rb2 {

using ColorType = bool;
static constexpr ColorType kColorRed = false;
static constexpr ColorType kColorBlack = true;

struct RBNodeBase;
using BasePtr       = RBNodeBase*;
using ConstBasePtr  = const RBNodeBase*;

struct RBNodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;
  ColorType color;

  inline bool is_red() const noexcept { return color == kColorRed; }
  inline bool is_black() const noexcept { return color == kColorBlack; }
  inline void set_red() noexcept { color = kColorRed; }
  inline void set_black() noexcept { color = kColorBlack; }

  static inline BasePtr minimum(BasePtr x) noexcept {
    while (x->left != nullptr)
      x = x->left;
    return x;
  }

  static inline ConstBasePtr minimum(ConstBasePtr x) noexcept {
    return minimum(const_cast<BasePtr>(x));
  }

  static inline BasePtr maximum(BasePtr x) noexcept {
    while (x->right != nullptr)
      x = x->right;
    return x;
  }

  static inline ConstBasePtr maximum(ConstBasePtr x) noexcept {
    return maximum(const_cast<BasePtr>(x));
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

  static inline ConstBasePtr successor(ConstBasePtr x) noexcept {
    return successor(const_cast<BasePtr>(x));
  }

  static BasePtr predecessor(BasePtr x) noexcept {
    if (x->color == kColorRed && x->parent->parent == x) {
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

  static inline ConstBasePtr predecessor(ConstBasePtr x) noexcept {
    return predecessor(const_cast<BasePtr>(x));
  }
};

namespace details {
  inline void transplant(BasePtr u, BasePtr v, BasePtr& root) noexcept {
    if (u->parent == nullptr)
      root = v;
    else if (u->parent->left == u)
      u->parent->left = v;
    else
      u->parent->right = v;
  }

  inline void left_rotate(BasePtr x, BasePtr& root) noexcept {
    //        |                   |
    //        x                   y
    //         \                 / \
    //          y               x   b
    //         / \               \
    //        a   b               a

    BasePtr y = x->right;
    x->right = y->left;
    if (x->right != nullptr)
      x->right->parent = x;
    y->parent = x->parent;
    transplant(x, y, root);
    y->left = x;
    x->parent = y;
  }

  inline void right_rotate(BasePtr x, BasePtr& root) noexcept {
    //        |                   |
    //        x                   y
    //       /                   / \
    //      y                   a   x
    //     / \                     /
    //    a   b                   b

    BasePtr y = x->left;
    x->left = y->right;
    if (x->left != nullptr)
      x->left->parent = x;
    y->parent = x->parent;
    transplant(x, y, root);
    y->right = x;
    x->parent = y;
  }

  inline void insert_fixup(BasePtr x, BasePtr& root) noexcept {
    while (x != root && x->parent->is_red()) {
      if (x->parent == x->parent->parent->left) {
        BasePtr y = x->parent->parent->right;
        if (y != nullptr && y->is_red()) {
          x->parent->set_black();
          y->set_black();
          x->parent->parent->set_red();
          x = x->parent->parent;
        }
        else {
          if (x == x->parent->right) {
            x = x->parent;
            left_rotate(x, root);
          }
          x->parent->set_black();
          x->parent->parent->set_red();
          right_rotate(x->parent->parent, root);
        }
      }
      else {
        BasePtr y = x->parent->parent->left;
        if (y != nullptr && y->is_red()) {
          x->parent->set_black();
          y->set_black();
          x->parent->parent->set_red();
          x = x->parent->parent;
        }
        else {
          if (x == x->parent->left) {
            x = x->parent;
            right_rotate(x, root);
          }
          x->parent->set_black();
          x->parent->parent->set_red();
          left_rotate(x->parent->parent, root);
        }
      }
    }
    root->set_black();
  }

  inline void insert(
      bool insert_left, BasePtr x, BasePtr p, RBNodeBase& header) noexcept {
    BasePtr& root = header.parent;

    x->parent = p;
    x->left = x->right = nullptr;
    x->set_red();

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

    insert_fixup(x, root);
  }

  inline void erase_fixup(BasePtr x, BasePtr& root) noexcept {
    while (x != root && (x == nullptr || x->is_black())) {
      if (x == x->parent->left) {
        BasePtr w = x->parent->right;
        if (w->is_red()) {
          w->set_black();
          x->parent->set_red();
          left_rotate(x->parent, root);
          w = x->parent->right;
        }

        if (w->left->is_black() && w->right->is_black()) {
          w->set_red();
          x = x->parent;
        }
        else {
          if (w->right->is_black()) {
            w->left->set_black();
            w->set_red();
            right_rotate(w, root);
            w = x->parent->right;
          }
          w->color = x->parent->color;
          x->parent->set_black();
          w->right->set_black();
          left_rotate(x->parent, root);
          x = root;
        }
      }
      else {
        BasePtr w = x->parent->left;
        if (w->is_red()) {
          w->set_black();
          x->parent->set_red();
          right_rotate(x->parent, root);
          w = x->parent->left;
        }

        if (w->left->is_black() && w->right->is_black()) {
          w->set_red();
          x = x->parent;
        }
        else {
          if (w->left->is_black()) {
            w->right->set_black();
            w->set_red();
            left_rotate(w, root);
            w = x->parent->left;
          }
          w->color = x->parent->color;
          x->parent->set_black();
          w->left->set_black();
          right_rotate(x->parent, root);
          x = root;
        }
      }
    }
    x->set_black();
  }

  inline void erase_transplant(BasePtr u, BasePtr v, BasePtr& root) noexcept {
    transplant(u, v, root);
    v->parent = u->parent;
  }

  inline void erase(BasePtr z, RBNodeBase& header) noexcept {
    BasePtr& root = header.parent;
    BasePtr& lmost = header.left;
    BasePtr& rmost = header.right;

    BasePtr y = z;
    BasePtr x = nullptr;
    if (y->left == nullptr) {
      x = y->right;
    }
    else if (y->right == nullptr) {
      x = y->left;
    }
    else {
      y = RBNodeBase::minimum(y->right);
      x = y->right;
    }

    if (y != z) {
      y->left = z->left;
      y->left->parent = y;
      if (y != z->right) {
        if (x != nullptr)
          x->parent = y->parent;
        y->parent->left = x;
        y->right = z->right;
        z->right->parent = y;
      }

      erase_transplant(z, y, root);
      std::swap(y->color, z->color);
      y = z;
    }
    else {
      if (x != nullptr)
        x->parent = y->parent;
      transplant(z, x, root);

      if (lmost == z)
        lmost = z->right == nullptr ? z->parent : RBNodeBase::minimum(x);
      if (rmost == z)
        rmost = z->left == nullptr ? z->parent : RBNodeBase::maximum(x);
    }

    if (y->is_black())
      erase_fixup(x, root);
  }
}

template <typename Value> struct RBNode : public RBNodeBase {
  Value value;
};

struct RBIterBase {
  BasePtr _node{};

  RBIterBase() noexcept {}
  RBIterBase(BasePtr x) noexcept : _node(x) {}
  RBIterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

  inline void increment() noexcept { _node = RBNodeBase::successor(_node); }
  inline void decrement() noexcept { _node = RBNodeBase::predecessor(_node); }
};

template <typename _Tp, typename _Ref, typename _Ptr>
struct RBIter : public RBIterBase {
  using Iter  = RBIter<_Tp, _Tp&, _Tp*>;
  using Self  = RBIter<_Tp, _Ref, _Ptr>;
  using Ref   = _Ref;
  using Ptr   = _Ptr;
  using Link  = RBNode<_Tp>*;

  RBIter() noexcept {}
  RBIter(BasePtr x) noexcept : RBIterBase(x) {}
  RBIter(ConstBasePtr x) noexcept : RBIterBase(x) {}
  RBIter(const Iter& x) noexcept : RBIterBase(x._node) {}

  inline Link node() noexcept { return Link(_node); }
  inline bool operator==(const Self& x) const noexcept { return _node == x._node; }
  inline bool operator!=(const Self& x) const noexcept { return _node != x._node; }
  inline Ref operator*() const noexcept { return Link(_node)->value; }
  inline Ptr operator->() const noexcept { return &Link(_node)->value; }

  Self& operator++() noexcept { increment(); return *this; }
  Self operator++(int) noexcept { Self tmp(*this); increment(); return tmp; }
  Self& operator--() noexcept { decrement(); return *this; }
  Self operator--(int) noexcept { Self tmp(*this); decrement(); return tmp; }
};

template <typename Tp,
         typename Less = std::less<Tp>, typename Equal = std::equal_to<Tp>>
class RBTree final : private UnCopyable {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = RBIter<Tp, Tp&, Tp*>;
  using ConstIter = RBIter<Tp, const Tp&, const Tp*>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
private:
  using Node      = RBNode<ValueType>;
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = Xt::SimpleAlloc<Node>;

  SizeType size_{};
  Node head_{};
  Less lt_fn_{};
  Equal eq_fn_{};

  static inline Link _parent(BasePtr x) noexcept { return Link(x->parent); }
  static inline ConstLink _parent(ConstBasePtr x) noexcept { return ConstLink(x->parent); }
  static inline Link _left(BasePtr x) noexcept { return Link(x->left); }
  static inline ConstLink _left(ConstBasePtr x) noexcept { return ConstLink(x->left); }
  static inline Link _right(BasePtr x) noexcept { return Link(x->right); }
  static inline ConstLink _right(ConstBasePtr x) noexcept { return ConstLink(x->right); }

  inline Link& root() noexcept { return (Link&)head_.parent; }
  inline ConstLink& root() const noexcept { return (ConstLink&)head_.parent; }
  inline Link& lmost() noexcept { return (Link&)head_.left; }
  inline ConstLink& lmost() const noexcept { return (ConstLink&)head_.left; }
  inline Link& rmost() noexcept { return (Link&)head_.right; }
  inline ConstLink& rmost() const noexcept { return (ConstLink&)head_.right; }

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

  inline Link create_node(ValueType&& value) {
    Link tmp = get_node();
    try {
      Xt::construct(&tmp->value, std::move(value));
    }
    catch (...) {
      put_node(tmp);
      throw;
    }
    return tmp;
  }

  template <typename... Args> inline Link create_node(Args&&... args) {
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

  inline void destroy_node(Link p) {
    Xt::destroy(&p->value);
    put_node(p);
  }

  inline std::tuple<bool, Link, bool> find_insert_pos(const ValueType& value) {
    Link x = root();
    Link p = &head_;
    while (x != nullptr) {
      if (eq_fn_(value, x->value))
        return std::make_tuple(false, nullptr, false);

      p = x;
      x = lt_fn_(value, x->value) ? _left(x) : _right(x);
    }
    bool insert_left = x != nullptr || p == &head_ || lt_fn_(value, p->value);

    return std::make_tuple(true, p, insert_left);
  }

  inline void insert_aux(const ValueType& value) {
    auto [r, p, insert_left] = find_insert_pos(value);
    if (r) {
      details::insert(insert_left, create_node(value), p, head_);
      ++size_;
    }
  }

  inline void insert_aux(ValueType&& value) {
    auto [r, p, insert_left] = find_insert_pos(value);
    if (r) {
      details::insert(insert_left, create_node(std::move(value)), p, head_);
      ++size_;
    }
  }

  template <typename... Args> inline void insert_aux(Args&&... args) {
    Link x = create_node(std::forward<Args>(args)...);
    auto [r, p, insert_left] = find_insert_pos(x->value);
    if (r) {
      details::insert(insert_left, x, p, head_);
      ++size_;
    }
    else {
      destroy_node(x);
    }
  }

  inline void erase_aux(Link p) {
    if (!empty()) {
      details::erase(p, head_);
      destroy_node(p);
      --size_;
    }
  }

  inline ConstLink find_aux(const ValueType& key) const noexcept {
    ConstLink x = root();
    ConstLink y = &head_;
    while (x != nullptr) {
      if (eq_fn_(key, x->value)) {
        y = x;
        break;
      }

      x = lt_fn_(key, x->value) ? _left(x) : _right(x);
    }
    return y;
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

  void clear() {
    _tear_node(root());
    init();
  }

  void insert(const ValueType& x) { insert_aux(x); }
  void insert(ValueType&& x) { insert_aux(std::move(x)); }

  template <typename... Args> void insert(Args&&... args) {
    insert_aux(std::forward<Args>(args)...);
  }

  void erase(ConstIter pos) { erase_aux(pos.node()); }

  Iter find(const ValueType& key) noexcept { return find_aux(key); }
  ConstIter find(const ValueType& key) const noexcept { return find_aux(key); }

  template <typename Function> inline void for_each(Function&& fn) {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }
};

}

void test_rb2() {
  rb2::RBTree<int> t;
  auto show_rb = [&t] {
    std::cout << std::endl << "rb-tree size: " << t.size() << std::endl;
    if (!t.empty()) {
      std::cout << "rb-tree {"
                << "head: " << t.get_head() << ", "
                << "tail: " << t.get_tail() << "}"
                << std::endl;
    }
    t.for_each([](int value) {
        std::cout << "rb-tree (beg->end) item value: " << value << std::endl;
        });

    for (auto i = t.end(); i != t.begin();)
      std::cout << "rb-tree (end->beg) item value: " << *(--i) << std::endl;
  };

  t.insert(45);
  t.insert(34);
  t.insert(89);
  t.insert(77);
  t.insert(234);
  t.insert(9);
  t.insert(-56);
  show_rb();

  t.erase(t.begin());
  t.erase(--t.end());
  t.erase(t.begin());
  show_rb();

  std::cout << "find 89: " << (t.find(89) != t.end()) << std::endl;

  t.clear();
  show_rb();
}
