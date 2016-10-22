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
#ifndef __TYR_BASIC_CIRCULARBUFFER_HEADER_H__
#define __TYR_BASIC_CIRCULARBUFFER_HEADER_H__

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace tyr { namespace basic {

template <typename T, typename ElemType = typename T::value_type>
class CircularBufferIterator {
public:
  typedef CircularBufferIterator<T, ElemType>           SelfType;
  typedef T                                             CircularBufferType;
  typedef std::random_access_iterator_tag               iterator_category;
  typedef typename CircularBufferType::value_type       value_type;
  typedef typename CircularBufferType::size_type        size_type;
  typedef typename CircularBufferType::pointer          pointer;
  typedef typename CircularBufferType::const_pointer    const_pointer;
  typedef typename CircularBufferType::reference        reference;
  typedef typename CircularBufferType::const_reference  const_reference;
  typedef typename CircularBufferType::difference_type  difference_type;
private:
  CircularBufferType* buf_{nullptr};
  size_type pos_{0};
public:
  CircularBufferIterator(CircularBufferType* buf, size_type pos)
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
    return pos_ - r.pos_;
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

template <typename T, typename Alloc = std::allocator<T>>
class CircularBuffer {
public:
  typedef CircularBuffer<T, Alloc>                                  SelfType;
  typedef Alloc                                                     allocator_type;
  typedef typename Alloc::value_type                                value_type;
  typedef typename Alloc::size_type                                 size_type;
  typedef typename Alloc::pointer                                   pointer;
  typedef typename Alloc::const_pointer                             const_pointer;
  typedef typename Alloc::reference                                 reference;
  typedef typename Alloc::const_reference                           const_reference;
  typedef typename Alloc::difference_type                           difference_type;
  typedef CircularBufferIterator<SelfType>                          iterator;
  typedef CircularBufferIterator<const SelfType, const value_type>  const_iterator;
  typedef std::reverse_iterator<iterator>                           reverse_iterator;
  typedef std::reverse_iterator<const_iterator>                     const_reverse_iterator;

  enum {DEFAULT_CAPACITY = 64};
private:
  pointer buff_{nullptr};
  size_type capacity_{0};
  size_type head_{1};
  size_type tail_{0};
  size_type size_{0};
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
      throw std::out_of_range("`i` index out of range");
    return at_unchecked(i);
  }

  size_type next_tail(void) {
    return (tail_ + 1 == capacity_) ? 0 : tail_ + 1;
  }

  void increment_head(void) {
    ++head_;
    --size_;
    if (head_ == capacity_)
      head_ = 0;
  }

  void increment_tail(void) {
    ++size_;
    tail_ = next_tail();
  }

  template <typename ForwardIterator>
  void assign_n(ForwardIterator from, ForwardIterator to) {
    if (!empty())
      clear();
    while (from != to)
      push_back(*from++);
  }

  template <typename ForwardIterator>
  void reserve_assign_n(ForwardIterator from, ForwardIterator to) {
    if (!empty())
      clear();

    while (from != to) {
      if (full())
        reserve(static_cast<size_type>(capacity_ < 2 ? 2 : capacity_ * 1.5));
      push_back(*from++);
    }
  }

  void destroy_all(void) {
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

  CircularBuffer(const CircularBuffer<T, Alloc>& r)
    : capacity_(r.capacity_)
    , head_(r.head_)
    , tail_(r.tail_)
    , size_(r.size_)
    , alloc_(r.get_allocator()) {
    try {
      buff_ = alloc_.allocate(capacity_);
      assign_n(r.begin(), r.end());
    }
    catch (...) {
      destroy_all();
      alloc_.deallocate(buff_, capacity_);
      throw;
    }
  }

  template <typename InputIterator>
  CircularBuffer(InputIterator from, InputIterator to, const allocator_type& alloc = allocator_type())
    : alloc_(alloc) {
    SelfType tmp(alloc);
    tmp.reserve_assign_n(from, to);
    swap(tmp);

    // FIXME: another implementation
    // SelfType tmp(std::distance(from, to), alloc);
    // tmp.assign_n(from, to);
    // swap(tmp);
  }

  ~CircularBuffer(void) {
    destroy_all();
    alloc_.deallocate(buff_, capacity_);
  }

  CircularBuffer(CircularBuffer<T, Alloc>&& r)
    : alloc_(r.get_allocator()) {
    r.swap(*this);
  }

  SelfType& operator=(const CircularBuffer<T, Alloc>& r) {
    CircularBuffer tmp(r);
    swap(tmp);
    return *this;
  }

  SelfType& operator=(CircularBuffer<T, Alloc>&& r) {
    r.swap(*this);
    CircularBuffer<T, Alloc>(get_allocator()).swap(r);
    return *this;
  }

  allocator_type get_allocator(void) const {
    return alloc_;
  }

  void swap(CircularBuffer<T, Alloc>& r) {
    std::swap(buff_, r.buff_);
    std::swap(capacity_, r.capacity_);
    std::swap(head_, r.head_);
    std::swap(tail_, r.tail_);
    std::swap(size_, r.size_);
    std::swap(alloc_, r.alloc_);
  }

  void reserve(size_type new_size) {
    if (capacity() < new_size) {
      SelfType tmp(new_size);
      tmp.assign_n(begin(), end());
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
    size_type pos = head_;
    increment_head();
    alloc_.destroy(buff_ + pos);
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

template <typename T, typename Alloc>
inline bool operator==(const CircularBuffer<T, Alloc>& a, const CircularBuffer<T, Alloc>& b) {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, typename Alloc>
inline bool operator!=(const CircularBuffer<T, Alloc>& a, const CircularBuffer<T, Alloc>& b) {
  return a.size() != b.size() || !std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, typename Alloc>
inline bool operator<(const CircularBuffer<T, Alloc>& a, const CircularBuffer<T, Alloc>& b) {
  return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

}}

#endif // __TYR_BASIC_CIRCULARBUFFER_HEADER_H__
