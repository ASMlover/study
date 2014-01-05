#ifndef __LOCKER_HEADER_H__
#define __LOCKER_HEADER_H__

template <typename Locker>
class LockerGuard : private UnCopyable {
  Locker& locker_;
public:
  explicit LockerGuard(Locker& locker)
    : locker_(locker)
  {
    locker_.Lock();
  }

  ~LockerGuard(void)
  {
    locker_.Unlock();
  }
};

#endif  //! __LOCKER_HEADER_H__
