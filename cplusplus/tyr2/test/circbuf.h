#ifndef CIRCBUF_HEADER_H
#define CIRCBUF_HEADER_H

#include <algorithm>
#include <memory>
#include <stdexcept>

template <typename T, typename elem_type = typename T::value_type>
class circbuf_iterator {
public:
  typedef circbuf_iterator<T, elem_type>          self_type;
  typedef T                                       circbuf_type;
  typedef std::random_access_iterator_tag         iterator_category;
  typedef typename circbuf_type::value_type       value_type;
  typedef typename circbuf_type::size_type        size_type;
  typedef typename circbuf_type::pointer          pointer;
  typedef typename circbuf_type::const_pointer    const_pointer;
  typedef typename circbuf_type::reference        reference;
  typedef typename circbuf_type::const_reference  const_reference;
  typedef typename circbuf_type::difference_type  difference_type;
private:
  circbuf_type* buf_{nullptr};
  size_type pos_{0};
public:
  circbuf_iterator(circbuf_type* buf, size_type pos)
    : buf_(buf)
    , pos_(pos) {
  }

  circbuf_iterator(const circbuf_iterator<T, elem_type>& r)
    : buf_(r.buf_)
    , pos_(r.pos_) {
  }

  elem_type& operator*(void) {
    return (*buf_)[pos_];
  }

  elem_type* operator->(void) {
    return &(operator*());
  }

  self_type& operator++(void) {
    ++pos_;
    return *this;
  }

  self_type operator++(int) {
    self_type tmp(*this);
    ++(*this);
    return tmp;
  }

  self_type& operator--(void) {
    --pos_;
    return *this;
  }

  self_type operator--(int) {
    self_type tmp(*this);
    --(*this);
    return tmp;
  }

  self_type operator+(difference_type n) const {
    self_type tmp(*this);
    tmp.pos_ += n;
    return tmp;
  }

  self_type& operator+=(difference_type n) {
    pos_ += n;
    return *this;
  }

  self_type operator-(difference_type n) const {
    self_type tmp(*this);
    tmp.pos_ -= n;
    return tmp;
  }

  self_type& operator-=(difference_type n) {
    pos_ -= n;
    return *this;
  }

  difference_type operator-(const circbuf_iterator<T, elem_type>& r) const {
    return pos_ - r.pos_;
  }

  bool operator==(const circbuf_iterator<T, elem_type>& r) const {
    return pos_ == r.pos_ && buf_ == r.buf_;
  }

  bool operator!=(const circbuf_iterator<T, elem_type>& r) const {
    return pos_ != r.pos_ && buf_ == r.buf_;
  }

  bool operator<(const circbuf_iterator<T, elem_type>& r) const {
    return pos_ < r.pos_;
  }

  bool operator<=(const circbuf_iterator<T, elem_type>& r) const {
    return pos_ <= r.pos_;
  }

  bool operator>(const circbuf_iterator<T, elem_type>& r) const {
    return pos_ > r.pos_;
  }

  bool operator>=(const circbuf_iterator<T, elem_type>& r) const {
    return pos_ >= r.pos_;
  }
};

template <typename T, typename Alloc = std::allocator<T>>
class circbuf {
public:
  typedef circbuf<T, Alloc>                                   self_type;
  typedef Alloc                                               allocator_type;
  typedef typename Alloc::value_type                          value_type;
  typedef typename Alloc::size_type                           size_type;
  typedef typename Alloc::pointer                             pointer;
  typedef typename Alloc::const_pointer                       const_pointer;
  typedef typename Alloc::reference                           reference;
  typedef typename Alloc::const_reference                     const_reference;
  typedef typename Alloc::difference_type                     difference_type;
  typedef circbuf_iterator<self_type>                         iterator;
  typedef circbuf_iterator<const self_type, const value_type> const_iterator;
  typedef std::reverse_iterator<iterator>                     reverse_iterator;
  typedef std::reverse_iterator<const_iterator>               const_reverse_iterator;

  enum { DEFAULT_CAPACITY = 128 };
private:
  pointer buff_;
  size_type capacity_;
  size_type head_;
  size_type tail_;
  size_type size_;
  allocator_type alloc_;
public:
  explicit circbuf(size_type capacity = DEFAULT_CAPACITY)
    : buff_(alloc_.allocate(capacity))
    , capacity_(capacity)
    , head_(1)
    , tail_(0)
    , size_(0) {
  }

  circbuf(const circbuf<T, Alloc>& r)
    : buff_(alloc_.allocate(r.capacity_))
    , capacity_(r.capacity_)
    , head_(r.head_)
    , tail_(r.tail_)
    , size_(r.size_) {
    try {
      assign_n(r.begin(), r.end());
    }
    catch (...) {
      destroy_all_elements();
      alloc_.deallocate(buff_, capacity_);
      throw;
    }
  }

  template <typename InputIterator>
  circbuf(InputIterator from, InputIterator to)
    : buff_(alloc_.allocate(DEFAULT_CAPACITY))
    , capacity_(DEFAULT_CAPACITY)
    , head_(1)
    , tail_(0)
    , size_(0) {
    self_type tmp;
    tmp.assign_reserve_n(from, to);
    swap(tmp);
  }

  ~circbuf(void) {
    destroy_all_elements();
    alloc_.deallocate(buff_, capacity_);
  }

  self_type& operator=(const circbuf<T, Alloc>& r) {
    circbuf tmp(r);
    swap(tmp);
    return *this;
  }

  void swap(circbuf<T, Alloc>& r) {
    std::swap(buff_, r.buff_);
    std::swap(capacity_, r.capacity_);
    std::swap(head_, r.head_);
    std::swap(tail_, r.tail_);
    std::swap(size_, r.size_);
  }

  allocator_type get_allocator(void) const {
    return alloc_;
  }

  void clear(void) {
    for (size_type i = 0; i < size_; ++i)
      alloc_.destroy(buff_ + get_index(i));
    head_ = 1;
    tail_ = 0;
    size_ = 0;
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
    alloc_.deallocate(buff_ + pos);
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

  void reserve(size_type new_size) {
    if (capacity() < new_size) {
      self_type tmp(new_size);
      tmp.assign_n(begin(), end());
      swap(tmp);
    }
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
    reverse_iterator(end());
  }

  reverse_iterator rend(void) {
    reverse_iterator(begin());
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
private:
  reference at_unchecked(size_type i) const {
    return buff_[get_index(i)];
  }

  reference at_checked(size_type i) const {
    if (i >= size_)
      throw std::out_of_range();
    return at_unchecked(i);
  }

  size_type normalise(size_type n) const {
    return n % capacity_;
  }

  size_type get_index(size_type index) const {
    return normalise(index + head_);
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

  size_type next_tail(void) {
    return (tail_ + 1 == capacity_) ? 0 : tail_ + 1;
  }

  template <typename ForwardIterator>
  void assign_n(ForwardIterator from, ForwardIterator to) {
    if (0 != size_)
      clear();
    while (from != to) {
      push_back(*from);
      ++from;
    }
  }

  template <typename ForwardIterator>
  void assign_reserve_n(ForwardIterator from, ForwardIterator to) {
    if (0 != size_)
      clear();
    while (from != to) {
      if (full())
        reserve(static_cast<size_type>(capacity_ * 1.5));
      push_back(*from);
      ++from;
    }
  }

  void destroy_all_elements(void) {
    for (size_type i = 0; i < size_; ++i)
      alloc_.destroy(buff_ + get_index(i));
  }
};

template <typename T, typename Alloc>
inline bool operator==(const circbuf<T, Alloc>& a, const circbuf<T, Alloc>& b) {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, typename Alloc>
inline bool operator!=(const circbuf<T, Alloc>& a, const circbuf<T, Alloc>& b) {
  return a.size() != b.size() || !std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, typename Alloc>
inline bool operator<(const circbuf<T, Alloc>& a, const circbuf<T, Alloc>& b) {
  return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

#endif // CIRCBUF_HEADER_H
