// Copyright (c) 2014 ASMlover. All rights reserved.
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
#ifndef __UTIL_SMART_PTR_HEADER_H__
#define __UTIL_SMART_PTR_HEADER_H__


namespace util {


// abstruct base class for destructor of smart pointer
class RefPointer : private UnCopyable {
public:
  RefPointer(void) {}
  virtual ~RefPointer(void) {}

  virtual void Destroy(void) = 0;
};


template <typename T>
class RefPtrDelete : public RefPointer {
  T*  ptr_;
public:
  explicit RefPtrDelete(T* p)
    : ptr_(p) {
  }

  ~RefPtrDelete(void) {
  }

  virtual void Destroy(void) {
    if (NULL != ptr_)
      delete ptr_;
  }
};

template <typename T, typename D>
class RefPtrDestructor : public RefPointer {
  T*  ptr_;
  D   dtor_;
public:
  explicit RefPtrDestructor(T* p, D d)
    : ptr_(p)
    , dtor_(d) {
  }

  ~RefPtrDestructor(void) {
  }

  virtual void Destroy(void) {
    if (NULL != ptr_)
      dtor_(ptr_);
  }
};



// SmartPtr
//
// An enhanced relative of smart pointer with reference counted copy 
// semantics.
// This object pointed to is deleted when the last SmartPtr pointing 
// to it is destroyed or reset.
template <typename T, typename Locker = DummyLock> 
class SmartPtr {
  T*                  ptr_;
  RefPointer*         ref_ptr_;
  RefCounter<Locker>* ref_count_;

  typedef SmartPtr<T, Locker> SelfType;
public:
  SmartPtr(void) 
    : ptr_(NULL)
    , ref_ptr_(NULL)
    , ref_count_(NULL) {
  }

  explicit SmartPtr(T* p)
    : ptr_(p)
    , ref_ptr_(new RefPtrDelete<T>(ptr_))
    , ref_count_(new RefCounter<Locker>(1)) {
  }

  template <typename Y, typename D>
  explicit SmartPtr(Y* p, D d)
    : ptr_(p)
    , ref_ptr_(new RefPtrDestructor<Y, D>(ptr_, d))
    , ref_count_(new RefCounter<Locker>(1)) {
  }

  ~SmartPtr(void) {
    if (NULL != ptr_) {
      if (0 == --*ref_count_) {
        ref_ptr_->Destroy();
        ptr_ = NULL;
        delete ref_ptr_;
        ref_ptr_ = NULL;

        delete ref_count_;
        ref_count_ = NULL;
      }
    }
  }

  SmartPtr(const SmartPtr& x) 
    : ptr_(x.ptr_)
    , ref_ptr_(x.ref_ptr_)
    , ref_count_(x.ref_count_) {
    ++*ref_count_;
  }

  SmartPtr& operator=(const SmartPtr& x) {
    if (this != &x)
      SelfType(x).Swap(*this);

    return *this;
  }
public:
  T* Get(void) const {
    return ptr_;
  }

  void Reset(void) {
    SelfType().Swap(*this);
  }

  template <typename Y>
  void Reset(Y* p) {
    SelfType(p).Swap(*this);
  }

  template <typename Y, typename D>
  void Reset(Y* p, D d) {
    SelfType(p, d).Swap(*this);
  }

  T& operator*(void) const {
    return *ptr_;
  }

  T* operator->(void) const {
    return ptr_;
  }
private:
  void Swap(SmartPtr& x) {
    std::swap(ptr_, x.ptr_);
    std::swap(ref_ptr_, x.ref_ptr_);
    std::swap(ref_count_, x.ref_count_);
  }
};


}


#endif  // __UTIL_SMART_PTR_HEADER_H__
