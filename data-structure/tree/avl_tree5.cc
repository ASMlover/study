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
    BasePtr parent = node->parent;
    while (node == parent->right) {
      node = parent;
      parent = parent->parent;
    }
    if (node->right != parent)
      node = parent;
  }
  return node;
}

inline ConstBasePtr avlnode_next(ConstBasePtr node) noexcept {
  return avlnode_next(const_cast<BasePtr>(node));
}

inline BasePtr avlnode_prev(BasePtr node) noexcept {
  if (node == nullptr)
    return nullptr;

  if (node->parent->parent == node) {
    node = node->right;
  }
  else if (node->left != nullptr) {
    node = node->left;
    while (node->right != nullptr)
      node = node->right;
  }
  else {
    BasePtr parent = node->parent;
    while (node == parent->left) {
      node = parent;
      parent = parent->parent;
    }
    node = parent;
  }
  return node;
}

inline ConstBasePtr avlnode_prev(ConstBasePtr node) noexcept {
  return avlnode_prev(const_cast<BasePtr>(node));
}

inline void avlnode_replace_child(BasePtr old_node,
    BasePtr new_node, BasePtr parent, BasePtr& root) noexcept {
  if (parent != nullptr) {
    if (parent->left == old_node)
      parent->left = new_node;
    else
      parent->right = new_node;
  }
  else {
    root = new_node;
  }
}

inline void avlnode_replace(
    BasePtr old_node, BasePtr new_node, BasePtr& root) noexcept {
  avlnode_replace_child(old_node, new_node, old_node->parent, root);
  new_node->parent = old_node->parent;
  new_node->left = old_node->left;
  new_node->right = old_node->right;
  new_node->height = old_node->height;
  if (new_node->left != nullptr)
    new_node->left->parent = new_node;
  if (new_node->right != nullptr)
    new_node->right->parent = new_node;
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
  y->left = x;
  y->parent = x->parent;
  avlnode_replace_child(x, y, x->parent, root);
  x->parent = y;

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
  y->right = x;
  y->parent = x->parent;
  avlnode_replace_child(x, y, x->parent, root);
  x->parent = y;

  return y;
}

inline BasePtr avlnode_fix_left(BasePtr node, BasePtr& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //       [d]  b              [d]  c               /   \
  //           / \                 / \             a     b
  //          c  [g]              e   b           / \   / \
  //         / \                     / \        [d]  e f  [g]
  //        e   f                   f  [g]

  BasePtr right = node->right; // b = a->right
  if (right->left_height() > right->right_height()) {
    right = avlnode_rotate_right(right, root);
    right->right->update_height();
    right->update_height();
  }
  node = avlnode_rotate_left(node, root);
  node->left->update_height();
  node->update_height();

  return node;
}

inline BasePtr avlnode_fix_right(BasePtr node, BasePtr& root) noexcept {
  //          |                   |                   |
  //          a                   a                   c
  //         / \                 / \                 / \
  //        b  [g]              c  [g]              /   \
  //       / \                 / \                 b     a
  //     [d]  c               b   f               / \   / \
  //         / \             / \                [d]  e f  [g]
  //        e   f          [d]  e

  BasePtr left = node->left; // b = a->left
  if (left->left_height() < left->right_height()) {
    left = avlnode_rotate_left(left, root);
    left->left->update_height();
    left->update_height();
  }
  node = avlnode_rotate_right(node, root);
  node->right->update_height();
  node->update_height();

  return node;
}

inline void avlnode_rebalance(BasePtr node, BasePtr& root) noexcept {
  while (node != root) {
    int hl = node->left_height();
    int hr = node->right_height();
    int diff = hl - hr;
    int height = std::max(hl, hr) + 1;

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

inline void avltree_insert(
    bool insert_left, BasePtr x, BasePtr p, AVLNodeBase& header) noexcept {
  AVLNodeBase*& root = header.parent;

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

  inline Self& operator++() noexcept {
    _node = Node::from(avlnode_next(_node));
    return *this;
  }

  inline Self operator++(int) noexcept {
    Self r(*this);
    _node = Node::from(avlnode_next(_node));
    return r;
  }

  inline Self& operator--() noexcept {
    _node = Node::from(avlnode_prev(_node));
    return *this;
  }

  inline Self operator--(int) noexcept {
    Self r(*this);
    _node = Node::from(avlnode_prev(_node));
    return r;
  }
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

  inline void initialize() noexcept {
    head_.parent = nullptr;
    head_.left = head_.right = &head_;
    head_.height = 0;
  }

  inline Node* create_node(const ValueType& x) {
    Node* node = Alloc::allocate();
    try {
      construct(&node->value, x);
    }
    catch (...) {
      Alloc::deallocate(node);
      throw;
    }
    return node;
  }

  inline void destroy_node(Node* node) {
    destroy(&node->value);
    Alloc::deallocate(node);
  }

  inline void insert_aux(const ValueType& value) {
    Link x = _root_link();
    Link y = _tail_link();
    while (x != nullptr) {
      if (value == x->value)
        return;

      y = x;
      x = value < x->value ? _left(x) : _right(x);
    }

    bool insert_left = x != nullptr || y == _tail_link() || value < y->value;
    avltree_insert(insert_left, create_node(value), y, head_);
    ++size_;
  }
public:
  AVLTree() noexcept {
    initialize();
  }

  ~AVLTree() noexcept {}

  inline bool empty() const noexcept { return size_ == 0; }
  inline SizeType size() const noexcept { return size_; }
  inline Iter begin() noexcept { return Iter(head_.left); }
  inline ConstIter begin() const noexcept { return ConstIter(head_.left); }
  inline Iter end() noexcept { return Iter(&head_); }
  inline ConstIter end() const noexcept { return ConstIter(&head_); }
  inline Ref get_head() noexcept { return *begin(); }
  inline Ref get_tail() noexcept { return *(--end()); }

  template <typename Function> inline void for_each(Function&& fn) noexcept {
    for (auto i = begin(); i != end(); ++i)
      fn(*i);
  }

  inline void insert(const ValueType& x) { insert_aux(x); }
};

}

void test_avl5() {
  avl5::AVLTree<int> t;

  auto show_avl = [&t] {
    std::cout << "\navltree => {addr: "
      << &t << ", size: " << t.size() << "}" << std::endl;

    for (auto i = t.begin(); i != t.end(); ++i)
      std::cout << "avltree item value: " << *i << std::endl;

    if (!t.empty()) {
      std::cout << "avltree => {"
        << t.get_head() << ", " << t.get_tail() << "}" << std::endl;
    }
  };

  t.insert(45);
  t.insert(33);
  t.insert(232);
  t.insert(56);
  t.insert(8);
  show_avl();
}
