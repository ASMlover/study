// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_REF_HEADER_H__
#define __EL_REF_HEADER_H__

namespace el {

template <typename _Tp> class Ref {
  _Tp*  obj_;
  mutable const Ref<_Tp>* prev_;
  mutable const Ref<_Tp>* next_;
public:
  Ref(void)
    : obj_(nullptr)
    , prev_(this)
    , next_(this) {
  }

  explicit Ref(_Tp* obj)
    : obj_(obj)
    , prev_(this)
    , next_(this) {
  }

  Ref(const Ref<_Tp>& other)
    : obj_(nullptr)
    , prev_(this)
    , next_(this) {
    if (&other != this)
      Link(other);
  }

  ~Ref(void) {
    Clear();
  }

  inline void Clear(void) {
    if (next_ != this) {
      prev_->next_ = this;
      next_->prev_ = this;

      prev_ = this;
      next_ = this;
    }
    else if (nullptr != obj_) {
      delete obj_;
    }

    obj_ = nullptr;
  }

  inline bool IsNil(void) const {
    return (nullptr == obj_);
  }

  inline operator bool(void) const {
    return (nullptr != obj_);
  }

  inline _Tp& operator*(void) const {
    return *obj_;
  }

  inline _Tp* operator->(void) const {
    return obj_;
  }

  inline bool operator==(const Ref<_Tp>& other) const {
    return (obj_ == other.obj_);
  }

  inline bool operator!=(const Ref<_Tp>& other) const {
    return (obj_ != other.obj_);
  }

  inline Ref<_Tp>& operator=(const Ref<_Tp>& other) {
    if (&other != this) {
      Clear();
      Link(other);
    }

    return *this;
  }
private:
  void Link(const Ref<_Tp>& other) {
    if (nullptr != other.obj_) {
      obj_ = other.obj_;

      prev_ = other.prev_;
      next_ = &other;

      other.prev_ = this;
      prev_->next_ = this;
    }
  }
};

template <typename _Tp>
inline std::ostream& operator<<(std::ostream& cout, const Ref<_Tp>& ref) {
  if (!ref)
    cout << "(nilref)";
  else
    cout << *ref;

  return cout;
}

}

#endif  // __EL_REF_HEADER_H__
