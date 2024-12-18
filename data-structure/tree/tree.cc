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
#include <tuple>
#include <iostream>
#include "common.hh"

namespace tree {

using ColorType = bool;
static constexpr ColorType kColorRed = false;
static constexpr ColorType kColorBlack = true;
static constexpr int kHeightMask = 0xff;

struct NodeBase;
using BasePtr       = NodeBase*;
using ConstBasePtr  = const NodeBase*;

struct NodeBase {
  BasePtr parent;
  BasePtr left;
  BasePtr right;

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

  static ConstBasePtr successor(ConstBasePtr x) noexcept {
    return successor(const_cast<BasePtr>(x));
  }

  template <typename Checker>
  static BasePtr predecessor(BasePtr x, Checker&& checker) noexcept {
    if (checker(x)) {
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

  static ConstBasePtr predecessor(ConstBasePtr x) noexcept {
    return predecessor(const_cast<BasePtr>(x));
  }
};

struct AVLNodeBase : public NodeBase {
  int height;

  inline bool is_root() const noexcept {
    return height == kHeightMask && parent->parent == this;
  }

  inline int lheight() const noexcept {
    return left ? static_cast<AVLNodeBase*>(left)->height : 0;
  }

  inline int rheight() const noexcept {
    return right ? static_cast<AVLNodeBase*>(right)->height : 0;
  }

  inline void update_height() noexcept {
    height = Xt::max(lheight(), rheight()) + 1;
  }
};

struct RBNodeBase : public NodeBase {
  ColorType color;

  inline bool is_root() const noexcept {
    return color == kColorRed && parent->parent == this;
  }

  inline bool is_red() const noexcept { return color == kColorRed; }
  inline bool is_black() const noexcept { return color == kColorBlack; }
  inline void set_red() noexcept { color = kColorRed; }
  inline void set_black() noexcept { color = kColorBlack; }
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

  inline BasePtr left_rotate(BasePtr x, BasePtr& root) noexcept {
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

    return y;
  }

  inline BasePtr right_rotate(BasePtr x, BasePtr& root) noexcept {
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

    return y;
  }

  namespace avl {
    inline AVLNodeBase* left_fixup(AVLNodeBase* a, BasePtr& root) noexcept {
      //      |                   |                   |
      //      a                   a                   c
      //     / \                 / \                 / \
      //   [d]  b              [d]  c               /   \
      //       / \                 / \             a     b
      //      c  [g]              e   b           / \   / \
      //     / \                     / \        [d]  e b  [g]
      //    e   f                   f  [g]

      AVLNodeBase* b = (AVLNodeBase*)a->right;
      if (b->lheight() > b->rheight()) {
        b = (AVLNodeBase*)right_rotate(b, root);
        ((AVLNodeBase*)b->right)->update_height();
        b->update_height();
      }
      a = (AVLNodeBase*)left_rotate(a, root);
      ((AVLNodeBase*)a->left)->update_height();
      a->update_height();

      return a;
    }

    inline AVLNodeBase* right_fixup(AVLNodeBase* a, BasePtr& root) noexcept {
      //      |                   |                   |
      //      a                   a                   c
      //     / \                 / \                 / \
      //    b  [g]              c  [g]              /   \
      //   / \                 / \                 b     a
      // [d]  c               b   f               / \   / \
      //     / \             / \                [d]  e f  [g]
      //    e   f          [d]  e

      AVLNodeBase* b = (AVLNodeBase*)a->left;
      if (b->lheight() < b->rheight()) {
        b = (AVLNodeBase*)left_rotate(b, root);
        ((AVLNodeBase*)b->left)->update_height();
        b->update_height();
      }
      a = (AVLNodeBase*)right_rotate(a, root);
      ((AVLNodeBase*)a->right)->update_height();
      a->update_height();

      return a;
    }

    inline void fixup(BasePtr x, BasePtr& root) noexcept {
      AVLNodeBase* node = (AVLNodeBase*)x;
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
          node = left_fixup(node, root);
        else if (diff >= 2)
          node = right_fixup(node, root);

        node = (AVLNodeBase*)node->parent;
      }
    }

    inline void insert(
        bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept {
      BasePtr& root = header.parent;

      x->parent = p;
      x->left = x->right = nullptr;
      ((AVLNodeBase*)x)->height = 1;

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
      fixup(x, root);
    }

    inline void erase(BasePtr x, NodeBase& header) noexcept {
      BasePtr& root = header.parent;
      BasePtr& lmost = header.left;
      BasePtr& rmost = header.right;
      BasePtr z = x;

      BasePtr y = nullptr;
      BasePtr p = nullptr;
      if (x->left != nullptr && x->right != nullptr) {
        x = NodeBase::minimum(x->right);
        y = x->right;
        p = x->parent;
        if (y != nullptr)
          y->parent = p;
        transplant(x, y, root);
        if (x->parent == z)
          p = x;
        x->parent = z->parent;
        x->left = z->left;
        x->right = z->right;
        ((AVLNodeBase*)x)->height = ((AVLNodeBase*)z)->height;
        transplant(z, x, root);
        x->left->parent = x;
        if (x->right != nullptr)
          x->right->parent = x;
      }
      else {
        y = x->left != nullptr ? x->left : x->right;
        p = x->parent;
        transplant(x, y, root);
        if (y != nullptr)
          y->parent = p;
      }
      if (p != nullptr)
        fixup(p, root);

      if (root == z)
        root = y;

      if (lmost == z)
        lmost = z->right == nullptr ? z->parent : NodeBase::minimum(y);
      if (rmost == z)
        rmost = z->left == nullptr ? z->parent : NodeBase::maximum(y);
    }
  }

  namespace rb {
    inline void insert_fixup(BasePtr x, BasePtr& root) noexcept {
      while (x != root && ((RBNodeBase*)x->parent)->is_red()) {
        if (x->parent == x->parent->parent->left) {
          RBNodeBase* y = (RBNodeBase*)x->parent->parent->right;
          if (y != nullptr && y->is_red()) {
            ((RBNodeBase*)x->parent)->set_black();
            y->set_black();
            ((RBNodeBase*)x->parent->parent)->set_red();
            x = x->parent->parent;
          }
          else {
            if (x == x->parent->right) {
              x = x->parent;
              left_rotate(x, root);
            }
            ((RBNodeBase*)x->parent)->set_black();
            ((RBNodeBase*)x->parent->parent)->set_red();
            right_rotate(x->parent->parent, root);
          }
        }
        else {
          RBNodeBase* y = (RBNodeBase*)x->parent->parent->left;
          if (y != nullptr && y->is_red()) {
            ((RBNodeBase*)x->parent)->set_black();
            y->set_black();
            ((RBNodeBase*)x->parent->parent)->set_red();
            x = x->parent->parent;
          }
          else {
            if (x == x->parent->left) {
              x = x->parent;
              right_rotate(x, root);
            }
            ((RBNodeBase*)x->parent)->set_black();
            ((RBNodeBase*)x->parent->parent)->set_red();
            left_rotate(x->parent->parent, root);
          }
        }
      }
      ((RBNodeBase*)root)->set_black();
    }

    inline void insert(
        bool insert_left, BasePtr x, BasePtr p, NodeBase& header) noexcept {
      BasePtr& root = header.parent;

      x->parent = p;
      x->left = x->right = nullptr;
      ((RBNodeBase*)x)->set_red();

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

    inline void erase_fixup(BasePtr z, BasePtr& root) noexcept {
      RBNodeBase* x = (RBNodeBase*)z;
      while (x != root && (x == nullptr || x->is_black())) {
        if (x->parent->left == x) {
          RBNodeBase* w = (RBNodeBase*)x->parent->right;
          if (w->is_red()) {
            w->set_black();
            ((RBNodeBase*)x->parent)->set_red();
            left_rotate(x->parent, root);
            w = (RBNodeBase*)x->parent->right;
          }

          if (((RBNodeBase*)w->left)->is_black()
              && ((RBNodeBase*)w->right)->is_black()) {
            w->set_red();
            x = (RBNodeBase*)x->parent;
          }
          else {
            if (((RBNodeBase*)w->right)->is_black()) {
              ((RBNodeBase*)w->left)->set_black();
              w->set_red();
              right_rotate(w, root);
              w = (RBNodeBase*)x->parent->right;
            }
            w->color = ((RBNodeBase*)x->parent)->color;
            ((RBNodeBase*)x->parent)->set_black();
            ((RBNodeBase*)w->right)->set_black();
            left_rotate(x->parent, root);
            x = (RBNodeBase*)root;
          }
        }
        else {
          RBNodeBase* w = (RBNodeBase*)x->parent->left;
          if (w->is_red()) {
            w->set_black();
            ((RBNodeBase*)x->parent)->set_red();
            right_rotate(x->parent, root);
            w = (RBNodeBase*)x->parent->left;
          }

          if (((RBNodeBase*)w->left)->is_black()
              && ((RBNodeBase*)w->right)->is_black()) {
            w->set_red();
            x = (RBNodeBase*)x->parent;
          }
          else {
            if (((RBNodeBase*)w->left)->is_black()) {
              ((RBNodeBase*)w->right)->set_black();
              w->set_red();
              left_rotate(w, root);
              w = (RBNodeBase*)x->parent->left;
            }
            w->color = ((RBNodeBase*)x->parent)->color;
            ((RBNodeBase*)x->parent)->set_black();
            ((RBNodeBase*)w->left)->set_black();
            right_rotate(x->parent, root);
            x = (RBNodeBase*)root;
          }
        }
      }
      x->set_black();
    }

    inline void erase(BasePtr z, NodeBase& header) noexcept {
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
        y = NodeBase::minimum(y->right);
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
          y->right->parent = y;
        }
        transplant(z, y, root);
        y->parent = z->parent;
        std::swap(((RBNodeBase*)y)->color, ((RBNodeBase*)z)->color);
        y = z;
      }
      else {
        if (x != nullptr)
          x->parent = y->parent;
        transplant(z, x, root);

        if (lmost == z)
          lmost = z->right == nullptr ? z->parent : NodeBase::minimum(x);
        if (rmost == z)
          rmost = z->left == nullptr ? z->parent : NodeBase::maximum(x);
      }

      if (((RBNodeBase*)y)->is_black())
        erase_fixup(x, root);
    }
  }
}

template <typename Value> struct AVLNode : public AVLNodeBase {
  Value value;
};

template <typename Value> struct RBNode : public RBNodeBase {
  Value value;
};

struct IterBase {
  NodeBase* _node{};

  IterBase() noexcept {}
  IterBase(BasePtr x) noexcept : _node(x) {}
  IterBase(ConstBasePtr x) noexcept : _node(const_cast<BasePtr>(x)) {}

  inline bool operator==(const IterBase& x) const noexcept {
    return _node == x._node;
  }

  inline bool operator!=(const IterBase& x) const noexcept {
    return _node != x._node;
  }

  inline void increment() noexcept { _node = NodeBase::successor(_node); }

  template <typename Function> inline void decrement(Function&& fn) noexcept {
    _node = NodeBase::predecessor(_node, std::move(fn));
  }
};

template <typename _Tp, typename _Ref, typename _Ptr, typename _Node>
struct Iterator : public IterBase {
  using Iter = Iterator<_Tp, _Tp&, _Tp*, _Node>;
  using Self = Iterator<_Tp, _Ref, _Ptr, _Node>;
  using Ref  = _Ref;
  using Ptr  = _Ptr;
  using Link = _Node*;

  Iterator() noexcept {}
  Iterator(BasePtr x) noexcept : IterBase(x) {}
  Iterator(ConstBasePtr x) noexcept : IterBase(x) {}
  Iterator(const Iter& x) noexcept : IterBase(x._node) {}

  inline Link node() const noexcept { return Link(_node); }
  inline Ref operator*() const noexcept { return Link(_node)->value; }
  inline Ptr operator->() const noexcept { return &Link(_node)->value; }

  Self& operator++() noexcept { increment(); return *this; }
  Self operator++(int) noexcept { Self tmp(*this); increment(); return tmp; }

  Self& operator--() noexcept {
    decrement([](BasePtr x) -> bool { return Link(x)->is_root(); });
    return *this;
  }

  Self operator--(int) noexcept {
    Self tmp(*this);
    decrement([](BasePtr x) -> bool { return Link(x)->is_root(); });
    return tmp;
  }
};

template <typename Tp, typename Node,
         typename Less = std::less<Tp>, typename Equal = std::equal_to<Tp>>
class TreeBase {
public:
  using ValueType = Tp;
  using SizeType  = std::size_t;
  using Iter      = Iterator<Tp, Tp&, Tp*, Node>;
  using ConstIter = Iterator<Tp, const Tp&, const Tp*, Node>;
  using Ref       = Tp&;
  using ConstRef  = const Tp&;
protected:
  using Link      = Node*;
  using ConstLink = const Node*;
  using Alloc     = Xt::SimpleAlloc<Node>;

  SizeType size_{};
  Node head_{};
  Less lt_comp_{};
  Equal eq_comp_{};

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

  inline Link get_node() noexcept { return Alloc::allocate(); }
  inline void put_node(Link p) noexcept { Alloc::deallocate(p); }

  Link create_node(const ValueType& value) {
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

  Link create_node(ValueType&& value) {
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
    Link x = root();
    Link p = &head_;
    while (x != nullptr) {
      if (eq_comp_(value, x->value))
        return std::make_tuple(false, nullptr, false);

      p = x;
      x = lt_comp_(value, x->value) ? _left(x) : _right(x);
    }
    bool insert_left = x != nullptr || p == &head_ || lt_comp_(value, p->value);

    return std::make_tuple(true, p, insert_left);
  }

  template <typename Inserter>
  inline void insert_aux(Inserter&& insert_fn, const ValueType& value) {
    auto [r, p, insert_left] = find_insert_pos(value);
    if (r) {
      insert_fn(insert_left, create_node(value), p, head_);
      ++size_;
    }
  }

  template <typename Inserter>
  inline void insert_aux(Inserter&& insert_fn, ValueType&& value) {
    auto [r, p, insert_left] = find_insert_pos(value);
    if (r) {
      insert_fn(insert_left, create_node(std::move(value)), p, head_);
      ++size_;
    }
  }

  template <typename Inserter, typename... Args>
  inline void insert_aux(Inserter&& insert_fn, Args&&... args) {
    Link tmp = create_node(std::forward<Args>(args)...);
    auto [r, p, insert_left] = find_insert_pos(tmp->value);
    if (r) {
      insert_fn(insert_left, tmp, p, head_);
      ++size_;
    }
    else {
      destroy_node(tmp);
    }
  }

  template <typename Eraser>
  inline void erase_aux(Eraser&& erase_fn, BasePtr x) {
    Link p = Link(x);
    if (!empty()) {
      erase_fn(x, head_);
      destroy_node(p);
      --size_;
    }
  }

  void tear_subtree(Link x) {
    while (x != nullptr) {
      tear_subtree(_right(x));
      Link y = _left(x);
      destroy_node(x);
      x = y;
    }
  }

  inline ConstLink find_aux(const ValueType& key) const noexcept {
    ConstLink x = root();
    ConstLink y = &head_;
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

  Iter find(const ValueType& key) noexcept {
    return Iter(find_aux(key));
  }

  ConstIter find(const ValueType& key) const noexcept {
    return ConstIter(find_aux(key));
  }

  template <typename Function> inline void for_each(Function&& fn) {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }
};

template <typename Tp>
class AVLTree final : public TreeBase<Tp, AVLNode<Tp>>, private UnCopyable {
  using Base      = TreeBase<Tp, AVLNode<Tp>>;
  using ValueType = typename Base::ValueType;
  using Iter      = typename Base::Iter;
  using ConstIter = typename Base::ConstIter;

  inline void init() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.height = kHeightMask;
  }
public:
  AVLTree() noexcept { init(); }
  ~AVLTree() noexcept { clear(); }

  void clear() {
    tear_subtree(root());
    init();
  }

  void insert(const ValueType& x) {
    insert_aux(details::avl::insert, x);
  }

  void insert(ValueType&& x) {
    insert_aux(details::avl::insert, std::move(x));
  }

  template <typename... Args> void insert(Args&&... args) {
    insert_aux(details::avl::insert, std::forward<Args>(args)...);
  }

  void erase(ConstIter pos) {
    erase_aux(details::avl::erase, pos.node());
  }
};

template <typename Tp>
class RBTree final : public TreeBase<Tp, RBNode<Tp>>, private UnCopyable {
  using Base      = TreeBase<Tp, RBNode<Tp>>;
  using ValueType = typename Base::ValueType;
  using Iter      = typename Base::Iter;
  using ConstIter = typename Base::ConstIter;

  inline void init() noexcept {
    size_ = 0;
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.set_red();
  }
public:
  RBTree() noexcept { init(); }
  ~RBTree() noexcept { clear(); }

  void clear() {
    tear_subtree(root());
    init();
  }

  void insert(const ValueType& x) {
    insert_aux(details::rb::insert, x);
  }

  void insert(ValueType&& x) {
    insert_aux(details::rb::insert, std::move(x));
  }

  template <typename... Args> void insert(Args&&... args) {
    insert_aux(details::rb::insert, std::forward<Args>(args)...);
  }

  void erase(ConstIter pos) {
    erase_aux(details::rb::erase, pos.node());
  }
};

}

template <typename Tree> void show_tree(Tree& t, const std::string& s) {
  std::cout << "\n" << s << " size: " << t.size() << std::endl;
  if (!t.empty()) {
    std::cout << s << "{" << t.get_head() << ", " << t.get_tail() << "}" << std::endl;
  }
  for (auto i = t.begin(); i != t.end(); ++i)
    std::cout << s << " item value: " << *i << std::endl;
}

struct Value {
  int val_field{};

  Value(int x = 0) noexcept : val_field(x) {}

  bool operator==(Value x) const noexcept { return val_field == x.val_field; }
  bool operator<(Value x) const noexcept { return val_field < x.val_field; }
};

std::ostream& operator<<(std::ostream& out, Value v) noexcept {
  return out << "Value<" << v.val_field << ">";
}

void test_tree() {
  {
    tree::AVLTree<int> t;
    show_tree(t, "avltree");

    t.insert(34);
    t.insert(56);
    t.insert(23);
    t.insert(77);
    t.insert(19);
    t.insert(7);
    t.insert(66);
    t.insert(39);
    t.insert(93);
    show_tree(t, "avltree");

    t.erase(t.begin());
    t.erase(t.begin());
    t.erase(--t.end());
    show_tree(t, "avltree");

    auto pos = t.find(56);
    t.erase(pos);
    show_tree(t, "avltree");

    t.clear();
    show_tree(t, "avltree");
  }

  {
    tree::RBTree<int> t;
    show_tree(t, "rbtree");

    t.insert(34);
    t.insert(56);
    t.insert(23);
    t.insert(77);
    t.insert(19);
    t.insert(7);
    t.insert(66);
    t.insert(39);
    t.insert(93);
    show_tree(t, "rbtree");

    t.erase(t.begin());
    t.erase(t.begin());
    t.erase(--t.end());
    show_tree(t, "rbtree");

    auto pos = t.find(56);
    t.erase(pos);
    show_tree(t, "rbtree");

    t.clear();
    show_tree(t, "rbtree");
  }

  {
    tree::AVLTree<Value> t;
    t.insert(56);
    t.insert(78);
    t.insert(90);
    t.insert(34);
    t.insert(13);
    show_tree(t, "rbtree-value");

    t.clear();
    show_tree(t, "rbtree-value");
  }
}
