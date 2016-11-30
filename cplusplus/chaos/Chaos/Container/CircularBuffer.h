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
#ifndef CHAOS_CONTAINER_CIRCULARBUFFER_H
#define CHAOS_CONTAINER_CIRCULARBUFFER_H

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace Chaos {

template <typename T, typename ElemType = typename T::value_type>
class CircularBufferIterator {
public:
  typedef CircularBufferIterator<T, ElemType>             SelfType;
  typedef T                                               circualr_buffer_type;
  typedef std::random_access_iterator_tag                 iterator_category;
  typedef typename circualr_buffer_type::value_type       value_type;
  typedef typename circualr_buffer_type::size_type        size_type;
  typedef typename circualr_buffer_type::pointer          pointer;
  typedef typename circualr_buffer_type::const_pointer    const_pointer;
  typedef typename circualr_buffer_type::reference        reference;
  typedef typename circualr_buffer_type::const_reference  const_reference;
  typedef typename circualr_buffer_type::difference_type  difference_type;
private:
  circualr_buffer_type* buf_{};
  size_type pos_{};
public:
  CircularBufferIterator(circualr_buffer_type* buf, size_type pos)
    : buf_(buf)
    , pos_(pos) {
  }

  CircularBufferIterator(const CircularBufferIterator<T, ElemType>& r)
    : buf_(r.buf_)
    , pos_(r.pos_) {
  }

  SelfType& operator=(const CircularBufferIterator<T, ElemType>& r) {
    buf_ = r.buf_;
    pos_ = r.pos_;
    return *this;
  }

  ElemType& operator*(void) {
    return (*buf_)[pos_];
  }

  ElemType* operator->(void) {
    return &(operator*());
  }

  SelfType& operator++(void) {
    ++pos_;
    return *this;
  }

  SelfType operator++(int) {
    SelfType tmp(*this);
    ++(*this);
    return tmp;
  }

  SelfType& operator--(void) {
    --pos_;
    return *this;
  }

  SelfType operator--(int) {
    SelfType tmp(*this);
    --(*this);
    return tmp;
  }

  SelfType operator+(difference_type n) const {
    SelfType tmp(*this);
    tmp.pos_ += n;
    return tmp;
  }

  SelfType& operator+=(difference_type n) {
    pos_ += n;
    return *this;
  }

  SelfType operator-(difference_type n) const {
    SelfType tmp(*this);
    tmp.pos_ -= n;
    return tmp;
  }

  SelfType& operator-=(difference_type n) {
    pos_ -= n;
    return *this;
  }

  difference_type operator-(const CircularBufferIterator<T, ElemType>& r) const {
    return pos_ -= r.pos_;
  }

  bool operator==(const CircularBufferIterator<T, ElemType>& r) const {
    return pos_ == r.pos_ && buf_ == r.buf_;
  }

  bool operator!=(const CircularBufferIterator<T, ElemType>& r) const {
    return pos_ != r.pos_ && buf_ == r.buf_;
  }

  bool operator<(const CircularBufferIterator<T, ElemType>& r) const {
    return pos_ < r.pos_;
  }

  bool operator<=(const CircularBufferIterator<T, ElemType>& r) const {
    return pos_ <= r.pos_;
  }

  bool operator>(const CircularBufferIterator<T, ElemType>& r) const {
    return pos_ > r.pos_;
  }

  bool operator>=(const CircularBufferIterator<T, ElemType>& r) const {
    return pos_ >= r.pos_;
  }
};

template <typename T, typename Allocator = std::allocator<T>>
class CircularBuffer {
public:
  typedef CircularBuffer<T, Allocator>                              SelfType;
  typedef Allocator                                                 allocator_type;
  typedef typename allocator_type::value_type                       value_type;
  typedef typename allocator_type::size_type                        size_type;
  typedef typename allocator_type::pointer                          pointer;
  typedef typename allocator_type::const_pointer                    const_pointer;
  typedef typename allocator_type::reference                        reference;
  typedef typename allocator_type::const_reference                  const_reference;
  typedef typename allocator_type::difference_type                  difference_type;
  typedef CircularBufferIterator<SelfType>                          iterator;
  typedef CircularBufferIterator<const SelfType, const value_type>  const_iterator;
  typedef std::reverse_iterator<iterator>                           reverse_iterator;
  typedef std::reverse_iterator<const_iterator>                     const_reverse_iterator;
private:
  pointer buff_{};
  size_type capacity_{};
  size_type head_{1};
  size_type tail_{};
  size_type size_{};
  allocator_type alloc_;
private:
  size_type normalise(size_type i) const {
    return i % capacity_;
  }

  size_type get_index(size_type i) const {
    return normalise(i + head_);
  }

  reference at_unchecked(size_type i) const {
    return buff_[get_index(i)];
  }

  reference at_checked(size_type i) const {
    if (i >= size_)
      throw std::out_of_range("CircularBuffer::at_checked: index out of range");
    return at_unchecked(i);
  }

  size_type next_tail(size_type i) const {
    return (tail_ + 1 == capacity_) ? 0 : tail_ + 1;
  }

  void increment_head(void) {
    ++head_;
    --size_;
    if (head_ == capacity_)
      head_ = 0;
  }

  void increment_tail(void) {
    --size_;
    tail_ = next_tail();
  }

  template <typename InputIterator>
  void assign(InputIterator from, InputIterator to) {
    if (!empty())
      clear();
    while (from != to)
      push_back(*from++);
  }

  template <typename InputIterator>
  void reserve_assign(InputIterator from, InputIterator to) {
    if (!empty())
      clear();

    while (from != to) {
      if (full())
        reserve(static_cast<int>(capacity_ < 2 ? 2 : capacity_ * 1.5));
      push_back(*from++);
    }
  }

  void destroy(void) {
    for (size_type i = 0; i < size_; ++i)
      alloc_.destroy(buff_ + get_index(i));
  }
public:
  explicit CircularBuffer(const allocator_type& alloc = allocator_type())
    : alloc_(alloc) {
  }

  CircularBuffer(size_type capacity, const allocator_type& alloc = allocator_type())
    : capacity_(capacity)
    , alloc_(alloc) {
    buff_ = alloc_.allocate(capacity_);
  }

  template <typename InputIterator>
  CircularBuffer(InputIterator from, InputIterator to, const allocator_type& alloc = allocator_type())
    : alloc_(alloc) {
    SelfType tmp(std::distance(from, to));
    tmp.assign(from, to);
    swap(tmp);
  }

  CircularBuffer(const CircularBuffer<T, Allocator>& r)
    : capacity_(r.capacity_)
    , head_(r.head_)
    , tail_(r.tail_)
    , size_(r.size_)
    , alloc_(r.get_allocator()) {
    try {
      buff_ = alloc_.allocate(capacity_);
      assign(r.begin(), r.end());
    }
    catch (...) {
      destroy();
      alloc_.deallocate(buff_, capacity_);
      throw;
    }
  }

  CircularBuffer(CircularBuffer<T, Allocator>&& r)
    : alloc_(r.get_allocator()) {
    r.swap(*this);
  }

  ~CircularBuffer(void) {
    destroy();
    alloc_.deallocate(buff_, capacity_);
  }

  SelfType& operator=(const CircularBuffer<T, Allocator>& r) {
    SelfType tmp(r);
    swap(tmp);
    return *this;
  }

  SelfType& operator=(CircularBuffer<T, Allocator>&& r) {
    r.swap(*this);
    SelfType(get_allocator()).swap(r);
    return *this;
  }

  allocator_type get_allocator(void) const {
    return alloc_;
  }

  void swap(CircularBuffer<T, Allocator>& r) {
    std::swap(buff_, r.buff_);
    std::swap(capacity_, r.capacity_);
    std::swap(head_, r.head_);
    std::swap(tail_, r.tail_);
    std::swap(size_, r.size_);
    std::swap(alloc_, r.alloc_);
  }

  void reserve(size_type new_capacity) {
    if (capacity_ < new_capacity) {
      SelfType tmp(new_capacity);
      tmp.assign(begin(), end());
      swap(tmp);
    }
  }

  void push_back(const value_type& x) {
    size_type next = next_tail();
    if (size_ == capacity_) {
      buff_[next] = x;
      increment_head();
    }
    else {
      alloc_.construct(buff_ + next, x);
    }
    increment_tail();
  }

  void pop_front(void) {
    size_type remove_pos = head_;
    increment_head();
    alloc_.destroy(buff_ + remove_pos);
  }

  void clear(void) {
    for (size_type i = 0; i < size_; ++i)
      alloc_.destroy(buff_ + get_index(i));
    head_ = 1;
    tail_ = 0;
    size_ = 0;
  }

  explicit operator bool(void) const {
    return nullptr != buff_;
  }

  bool empty(void) const {
    return 0 == size_;
  }

  bool full(void) const {
    return size_ == capacity_;
  }

  size_type size(void) const {
    return size_;
  }

  size_type capacity(void) const {
    return capacity_;
  }

  size_type max_size(void) const {
    return alloc_.max_size();
  }

  pointer data(void) {
    return buff_;
  }

  const_pointer data(void) const {
    return buff_;
  }

  iterator begin(void) {
    return iterator(this, 0);
  }

  iterator end(void) {
    return iterator(this, size());
  }

  const_iterator begin(void) const {
    return const_iterator(this, 0);
  }

  const_iterator end(void) const {
    return const_iterator(this, size());
  }

  reverse_iterator rbegin(void) {
    return reverse_iterator(end());
  }

  reverse_iterator rend(void) {
    return reverse_iterator(begin());
  }

  const_reverse_iterator rbegin(void) const {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator rend(void) const {
    return const_reverse_iterator(begin());
  }

  reference front(void) {
    return buff_[head_];
  }

  reference back(void) {
    return buff_[tail_];
  }

  const_reference front(void) const {
    return buff_[head_];
  }

  const_reference back(void) const {
    return buff_[tail_];
  }

  reference operator[](size_type i) {
    return at_unchecked(i);
  }

  const_reference operator[](size_type i) const {
    return at_unchecked(i);
  }

  reference at(size_type i) {
    return at_checked(i);
  }

  const_reference at(size_type i) const {
    return at_checked(i);
  }
};

template <typename T, typename Allocator>
inline bool operator==(const CircularBuffer<T, Allocator>& a, const CircularBuffer<T, Allocator>& b) {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, typename Allocator>
inline bool operator!=(const CircularBuffer<T, Allocator>& a, const CircularBuffer<T, Allocator>& b) {
  return a.size() != b.size() || !std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, typename Allocator>
inline bool operator<(const CircularBuffer<T, Allocator>& a, const CircularBuffer<T, Allocator>& b) {
  return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

}

#endif // CHAOS_CONTAINER_CIRCULARBUFFER_H
