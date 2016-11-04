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
#ifndef TPP_BASIC_MEMORY_H_
#define TPP_BASIC_MEMORY_H_

#include <atomic>
#include <utility>

namespace tpp { namespace basic {

template <typename T>
class SharedPtr {
public:
  typedef T                   element_type;
  typedef element_type*       pointer;
  typedef const element_type* const_pointer;
  typedef element_type&       reference;
  typedef const element_type& const_reference;
private:
  struct _Container {
    pointer px;
    std::atomic<int> rc;

    explicit _Container(pointer p)
      : px(p)
      , rc(1) {
    }

    ~_Container(void) {
      delete px;
    }
  };
  _Container* pc_{nullptr};
public:
  SharedPtr(void) = default;

  explicit SharedPtr(pointer p)
    : pc_(new _Container(p)) {
  }

  SharedPtr(const SharedPtr& r)
    : pc_(r.p_) {
    if (nullptr != pc_)
      ++pc_->rc;
  }

  SharedPtr(SharedPtr&& r)
    : pc_(r.pc_) {
    r.pc_ = nullptr;
  }

  ~SharedPtr(void) {
    reset();
  }

  SharedPtr& operator=(pointer p) {
    reset(p);
    return *this;
  }

  SharedPtr& operator=(const SharedPtr& r) {
    reset();
    pc_ = r.pc_;
    if (nullptr != pc_)
      ++pc_->rc;
    return *this;
  }

  SharedPtr& operator=(SharedPtr&& r) {
    swap(r);
    return *this;
  }

  void reset(pointer p = nullptr) {
    auto old = pc_;
    pc_ = nullptr != p ? new _Container(p) : nullptr;
    if (nullptr != old && 0 == --old->rc)
      delete old;
  }

  void swap(SharedPtr& r) {
    std::swap(pc_, r.pc_);
  }

  explicit operator bool(void) const {
    return nullptr != get();
  }

  int use_count(void) const {
    return nullptr != pc_ ? pc_->rc : 0;
  }

  bool unique(void) const {
    return 1 == use_count();
  }

  pointer get(void) {
    return nullptr != pc_ ? pc_->px : nullptr;
  }

  const_pointer get(void) const {
    return nullptr != pc_ ? pc_->px : nullptr;
  }

  pointer operator->(void) {
    return get();
  }

  const_pointer operator->(void) const {
    return get();
  }

  reference operator*(void) {
    return *get();
  }

  const_reference operator*(void) const {
    return *get();
  }

  reference operator[](int i) {
    return get()[i];
  }

  const_reference operator[](int i) const {
    return get()[i];
  }

  bool operator==(pointer p) const {
    return get() == p;
  }

  bool operator==(const SharedPtr& r) const {
    return get() == r.get();
  }

  bool operator<(const SharedPtr& r) const {
    return get() < r.get();
  }
};

template <typename T, typename... Args>
inline SharedPtr<T> make_shared(Args&&... args) {
  return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

}}

#endif // TPP_BASIC_MEMORY_H_
