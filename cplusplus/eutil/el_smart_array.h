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
#ifndef __EL_SMART_ARRAY_HEADER_H__
#define __EL_SMART_ARRAY_HEADER_H__


namespace el {

class RefArray : private UnCopyable {
public:
  RefArray(void) {}
  virtual ~RefArray(void) {}

  virtual void Destroy(void) = 0;
};


template <typename T>
class RefArrayDelete : public RefArray {
  T* ptr_;
public:
  explicit RefArrayDelete(T* p) 
    : ptr_(p) {
  }

  ~RefArrayDelete(void) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      delete [] ptr_;
  }
};

template <typename T, typename D>
class RefArrayDestructor : public RefArray {
  T* ptr_;
  D  dtor_;
public:
  explicit RefArrayDestructor(T* p, D d) 
    : ptr_(p) 
    , dtor_(d) {
  }

  ~RefArrayDestructor(void) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      dtor_(ptr_);
  }
};



// SmartArray 
//
// SmartArray extends SmartPtr to arrays.
// The array pointed to is deleted when the last SmartArray 
// pointing to it is destroyed or reset.
template <typename T, typename Locker = DummyLock> 
class SmartArray {
  T*                  ptr_;
  RefArray*           ref_array_;
  RefCounter<Locker>* ref_count_;

  typedef SmartArray<T, Locker> SelfType;
public:
  SmartArray(void) 
    : ptr_(nullptr) 
    , ref_array_(nullptr) 
    , ref_count_(nullptr) {
  }

  template <typename Y> 
  explicit SmartArray(Y* p) 
    : ptr_(p) 
    , ref_array_(new RefArrayDelete<T>(ptr_)) 
    , ref_count_(new RefCounter<Locker>(1)) {
  }

  template <typename Y, typename D>
  explicit SmartArray(Y* p, D d) 
    : ptr_(p)
    , ref_array_(new RefArrayDestructor<T, D>(ptr_, d)) 
    , ref_count_(new RefCounter<Locker>(1)) {
  }

  ~SmartArray(void) {
    if (nullptr != ref_count_) {
      if (0 == --*ref_count_) {
        ref_array_->Destroy();
        ptr_ = nullptr;

        delete ref_array_;
        ref_array_ = nullptr;

        delete ref_count_;
        ref_count_ = nullptr;
      }
    }
  }

  SmartArray(const SmartArray& x) 
    : ptr_(x.ptr_) 
    , ref_array_(x.ref_array_) 
    , ref_count_(x.ref_count_) {
    if (nullptr != ref_count_)
      ++*ref_count_;
  }

  template <typename Y> 
  SmartArray(const SmartArray<Y, Locker>& x) 
    : ptr_(x.Get()) 
    , ref_array_(x.GetRefArray()) 
    , ref_count_(x.GetRefCounter()) {
    if (nullptr != ref_count_) 
      ++*ref_count_;
  }

  SmartArray& operator=(const SmartArray& x) {
    if (this != &x)
      SelfType(x).Swap(*this);

    return *this;
  }

  template <typename Y>
  SmartArray& operator=(const SmartArray<Y, Locker>& x) {
    if ((void*)this != (void*)&x)
      SelfType(x).Swap(*this);

    return *this;
  }
private:
  inline T* Get(void) const {
    return ptr_;
  }
  
  inline RefArray* GetRefArray(void) const {
    return ref_array_;
  }

  inline RefCounter<Locker>* GetRefCounter(void) const {
    return ref_count_;
  }

  inline void Reset(void) {
    SelfType().Swap(*this);
  }

  template <typename Y> 
  inline void Reset(Y* p) {
    SelfType(p).Swap(*this);
  }

  template <typename Y, typename D> 
  inline void Reset(Y* p, D d) {
    SelfType(p, d).Swap(*this);
  }

  inline T& operator[](uint32_t i) const {
    return ptr_[i];
  }
private:
  void Swap(SmartArray<T, Locker>& x) {
    std::swap(ptr_, x.ptr_);
    std::swap(ref_array_, x.ref_array_);
    std::swap(ref_count_, x.ref_count_);
  }
};

template <typename T, typename Y, typename Locker> 
inline bool operator==(
    const SmartArray<T, Locker>& x, const SmartArray<Y, Locker>& y) {
  return x.Get() == y.Get();
}

template <typename T, typename Y, typename Locker> 
inline bool operator!=(
    const SmartArray<T, Locker>& x, const SmartArray<Y, Locker>& y) {
  return x.Get() != y.Get();
}

}


#endif  // __EL_SMART_ARRAY_HEADER_H__
