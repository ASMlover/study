#ifndef __EL_REF_COUNTER_HEADER_H__
#define __EL_REF_COUNTER_HEADER_H__


namespace el {

template <typename Locker = DummyLock> 
class RefCounter : private UnCopyable {
  uint32_t counter_;
  Locker   locker_;
public:
  explicit RefCounter(uint32_t counter = 1) 
    : counter_(counter) {
  }

  ~RefCounter(void) {
  }

  inline operator uint32_t(void) const {
    return counter_;
  }

  inline uint32_t operator++(void) {
    LockerGuard<Locker> guard(locker_);
    return ++counter_;
  }

  inline uint32_t operator--(void) {
    LockerGuard<Locker> guard(locker_);
    return --counter_;
  }
};

}


#endif  // __EL_REF_COUNTER_HEADER_H__
