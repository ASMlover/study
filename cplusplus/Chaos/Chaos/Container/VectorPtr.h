// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef CHAOS_CONTAINER_VECTORPTR_H
#define CHAOS_CONTAINER_VECTORPTR_H

#include <vector>

namespace Chaos {

template <typename T, typename Allocator = std::allocator<T>>
class VectorPtr {
public:
  typedef std::vector<T, Allocator>                 vector_type;
  typedef T                                         value_type;
  typedef Allocator                                 allocator_type;
  typedef typename vector_type::size_type           size_type;
  typedef typename vector_type::difference_type     difference_type;
  typedef typename allocator_type::pointer          pointer;
  typedef typename allocator_type::const_pointer    const_pointer;
  typedef typename allocator_type::reference        reference;
  typedef typename allocator_type::const_reference  const_reference;
  typedef typename vector_type::iterator            iterator;
  typedef typename vector_type::const_iterator      const_iterator;
  typedef std::reverse_iterator<iterator>           reverse_iterator;
  typedef std::reverse_iterator<const_iterator>     const_reverse_iterator;
private:
  vector_type* vec_{};
public:
  VectorPtr(void) = default;
  ~VectorPtr(void) = default;

  explicit VectorPtr(vector_type& vec)
    : vec_(&vec) {
  }

  VectorPtr(const VectorPtr& r)
    : vec_(r.vec_) {
  }

  VectorPtr(VectorPtr&& r)
    : vec_(r.vec_) {
    r.vec_ = nullptr;
  }

  VectorPtr& operator=(const VectorPtr& r) {
    vec_ = r.vec_;
    return *this;
  }

  VectorPtr& operator=(VectorPtr&& r) {
    vec_ = r.vec_;
    r.vec_ = nullptr;
    return *this;
  }

  void reset(vector_type* vec = nullptr) {
    vec_ = vec;
  }

  explicit operator bool(void) const {
    return nullptr != vec_;
  }

  allocator_type get_allocator(void) const {
    return vec_->get_allocator();
  }

  vector_type* get(void) {
    return vec_;
  }

  const vector_type* get(void) const {
    return vec_;
  }

  iterator begin(void) {
    return vec_->begin();
  }

  const_iterator begin(void) const {
    return vec_->begin();
  }

  iterator end(void) {
    return vec_->end();
  }

  const_iterator end(void) const {
    return vec_->end();
  }

  reverse_iterator rbegin(void) {
    return vec_->rbegin();
  }

  const_reverse_iterator rbegin(void) const {
    return vec_->rbegin();
  }

  reverse_iterator rend(void) {
    return vec_->rend();
  }

  const_reverse_iterator rend(void) const {
    return vec_->rend();
  }

  const_iterator cbegin(void) const {
    return vec_->cbegin();
  }

  const_iterator cend(void) const {
    return vec_->cend();
  }

  const_reverse_iterator crbegin(void) const {
    return vec_->crbegin();
  }

  const_reverse_iterator crend(void) const {
    return vec_->crend();
  }

  size_type size(void) const {
    return vec_->size();
  }

  size_type capacity(void) const {
    return vec_->capacity();
  }

  size_type max_size(void) const {
    return vec_->max_size();
  }

  bool empty(void) const {
    return vec_->empty();
  }

  void resize(size_type n) {
    vec_->resize(n);
  }

  void resize(size_type n, const value_type& v) {
    vec_->resize(n, v);
  }

  void reserve(size_type n) {
    vec_->reserve(n);
  }

  void shrink_to_fit(void) {
    vec_->shrink_to_fit();
  }

  reference operator[](size_type i) {
    return vec_->operator[](i);
  }

  const_reference operator[](size_type i) const {
    return vec_->operator[](i);
  }

  reference at(size_type i) {
    return vec_->at(i);
  }

  const_reference at(size_type i) const {
    return vec_->at(i);
  }

  reference front(void) {
    return vec_->front();
  }

  const_reference front(void) const {
    return vec_->front();
  }

  reference back(void) {
    return vec_->back();
  }

  const_reference back(void) const {
    return vec_->back();
  }

  value_type* data(void) {
    return vec_->data();
  }

  const value_type* data(void) const {
    return vec_->data();
  }

  template <typename InputIterator>
  void assign(InputIterator from, InputIterator to) {
    vec_->assign(from, to);
  }

  void assign(size_type n, const value_type& v) {
    vec_->assign(n, v);
  }

  void assign(std::initializer_list<value_type> il) {
    vec_->assign(il);
  }

  void push_back(const value_type& v) {
    vec_->push_back(v);
  }

  void push_back(value_type&& v) {
    vec_->push_back(std::forward<value_type>(v));
  }

  void pop_back(void) {
    vec_->pop_back();
  }

  iterator insert(const_iterator position, const value_type& v) {
    return vec_->insert(position, v);
  }

  iterator insert(const_iterator position, value_type&& v) {
    return vec_->insert(position, std::forward<value_type>(v));
  }

  iterator insert(const_iterator position, size_type n, const value_type& v) {
    return vec_->insert(position, n, v);
  }

  template <typename InputIterator>
  iterator insert(const_iterator position, InputIterator from, InputIterator to) {
    return vec_->insert(position, from, to);
  }

  iterator insert(const_iterator position, std::initializer_list<value_type> il) {
    return vec_->insert(position, il);
  }

  iterator erase(const_iterator position) {
    return vec_->erase(position);
  }

  iterator erase(const_iterator from, const_iterator to) {
    return vec_->erase(from, to);
  }

  void swap(vector_type& r) {
    vec_->swap(r);
  }

  void swap(VectorPtr& r) {
    std::swap(vec_, r.vec_);
  }

  void clear(void) {
    vec_->clear();
  }

  template <typename... Args>
  iterator emplace(const_iterator position, Args&&... args) {
    return vec_->emplace(position, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    vec_->emplace_back(std::forward<Args>(args)...);
  }
};

template <typename T, typename Allocator>
inline bool operator==(const VectorPtr<T, Allocator>& a, const VectorPtr<T, Allocator>& b) {
  return a.get() == b.get() || *a.get() == *b.get();
}

template <typename T, typename Allocator>
inline bool operator!=(const VectorPtr<T, Allocator>& a, const VectorPtr<T, Allocator>& b) {
  return a.get() != b.get() && *a.get() != *b.get();
}

template <typename T, typename Allocator>
inline bool operator<(const VectorPtr<T, Allocator>& a, const VectorPtr<T, Allocator>& b) {
  return *a.get() < *b.get();
}

template <typename T, typename Allocator>
inline bool operator<=(const VectorPtr<T, Allocator>& a, const VectorPtr<T, Allocator>& b) {
  return *a.get() <= *b.get();
}

template <typename T, typename Allocator>
inline bool operator>(const VectorPtr<T, Allocator>& a, const VectorPtr<T, Allocator>& b) {
  return *a.get() > *b.get();
}

template <typename T, typename Allocator>
inline bool operator>=(const VectorPtr<T, Allocator>& a, const VectorPtr<T, Allocator>& b) {
  return *a.get() >= *b.get();
}

template <typename T, typename Allocator = std::allocator<T>>
class ConstVectorPtr {
public:
  typedef std::vector<T, Allocator>                 vector_type;
  typedef T                                         value_type;
  typedef Allocator                                 allocator_type;
  typedef typename vector_type::size_type           size_type;
  typedef typename vector_type::difference_type     difference_type;
  typedef typename allocator_type::pointer          pointer;
  typedef typename allocator_type::const_pointer    const_pointer;
  typedef typename allocator_type::reference        reference;
  typedef typename allocator_type::const_reference  const_reference;
  typedef typename vector_type::iterator            iterator;
  typedef typename vector_type::const_iterator      const_iterator;
  typedef std::reverse_iterator<iterator>           reverse_iterator;
  typedef std::reverse_iterator<const_iterator>     const_reverse_iterator;
private:
  const vector_type* vec_{};
public:
  ConstVectorPtr(void) = default;
  ~ConstVectorPtr(void) = default;

  explicit ConstVectorPtr(const vector_type& vec)
    : vec_(&vec) {
  }

  ConstVectorPtr(const ConstVectorPtr& r)
    : vec_(r.vec_) {
  }

  ConstVectorPtr(ConstVectorPtr&& r)
    : vec_(r.vec_) {
    r.vec_ = nullptr;
  }

  ConstVectorPtr& operator=(const ConstVectorPtr& r) {
    vec_ = r.vec_;
    return *this;
  }

  ConstVectorPtr& operator=(ConstVectorPtr&& r) {
    vec_ = r.vec_;
    r.vec_ = nullptr;
    return *this;
  }

  allocator_type get_allocator(void) const {
    return vec_->get_allocator();
  }

  void reset(const vector_type* vec = nullptr) {
    vec_ = vec;
  }

  explicit operator bool(void) const {
    return nullptr != vec_;
  }

  const vector_type* get(void) const {
    return vec_;
  }

  const_iterator begin(void) const {
    return vec_->begin();
  }

  const_iterator end(void) const {
    return vec_->end();
  }

  const_reverse_iterator rbegin(void) const {
    return vec_->rbegin();
  }

  const_reverse_iterator rend(void) const {
    return vec_->rend();
  }

  const_iterator cbegin(void) const {
    return vec_->cbegin();
  }

  const_iterator cend(void) const {
    return vec_->cend();
  }

  const_reverse_iterator crbegin(void) const {
    return vec_->crbegin();
  }

  const_reverse_iterator crend(void) const {
    return vec_->crend();
  }

  bool empty(void) const {
    return vec_->empty();
  }

  size_type size(void) const {
    return vec_->size();
  }

  size_type capacity(void) const {
    return vec_->capacity();
  }

  size_type max_size(void) const {
    return vec_->max_size();
  }

  const_reference operator[](size_type i) const {
    return vec_->operator[](i);
  }

  const_reference at(size_type i) const {
    return vec_->at(i);
  }

  const_reference front(void) const {
    return vec_->front();
  }

  const_reference back(void) const {
    return vec_->back();
  }

  const vector_type* data(void) const {
    return vec_->data();
  }
};

template <typename T, typename Allocator>
inline bool operator==(const ConstVectorPtr<T, Allocator>& a, const ConstVectorPtr<T, Allocator>& b) {
  return a.get() == b.get() or *a.get() == *b.get();
}

template <typename T, typename Allocator>
inline bool operator!=(const ConstVectorPtr<T, Allocator>& a, const ConstVectorPtr<T, Allocator>& b) {
  return a.get() != b.get() && *a.get() != *b.get();
}

template <typename T, typename Allocator>
inline bool operator<(const ConstVectorPtr<T, Allocator>& a, const ConstVectorPtr<T, Allocator>& b) {
  return *a.get() < *b.get();
}

template <typename T, typename Allocator>
inline bool operator<=(const ConstVectorPtr<T, Allocator>& a, const ConstVectorPtr<T, Allocator>& b) {
  return *a.get() <= *b.get();
}

template <typename T, typename Allocator>
inline bool operator>(const ConstVectorPtr<T, Allocator>& a, const ConstVectorPtr<T, Allocator>& b) {
  return *a.get() > *b.get();
}

template <typename T, typename Allocator>
inline bool operator>=(const ConstVectorPtr<T, Allocator>& a, const ConstVectorPtr<T, Allocator>& b) {
  return *a.get() >= *b.get();
}

template <typename T, typename Allocator = std::allocator<T>>
inline VectorPtr<T, Allocator> make_vector_ptr(std::vector<T, Allocator>& vec) {
  return VectorPtr<T, Allocator>(&vec);
}

template <typename T, typename Allocator = std::allocator<T>>
inline ConstVectorPtr<T, Allocator> make_vector_ptr(const std::vector<T, Allocator>& vec) {
  return ConstVectorPtr<T, Allocator>(&vec);
}

}

#endif // CHAOS_CONTAINER_VECTORPTR_H
