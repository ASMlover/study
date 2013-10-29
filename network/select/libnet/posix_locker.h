#ifndef __POSIX_LOCKER_HEADER_H__
#define __POSIX_LOCKER_HEADER_H__

#include <pthread.h>
#include <stdlib.h>

class SpinLock {
  pthread_spinlock_t spinlock_;

  SpinLock(const SpinLock&);
  SpinLock& operator =(const SpinLock&);
public:
  explicit SpinLock(void)
  {
    if (0 != pthread_spin_init(&spinlock_, NULL))
      abort();
  }

  ~SpinLock(void)
  {
    if (0 != pthread_spin_destroy(&spinlock_))
      abort();
  }

  inline void Lock(void) 
  {
    if (0 != pthread_spin_lock(&spinlock_))
      abort();
  }

  inline void Unlock(void)
  {
    if (0 != pthread_spin_unlock(&spinlock_))
      abort();
  }
};

#endif  //! __POSIX_LOCKER_HEADER_H__
