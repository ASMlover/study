#ifndef __POSIX_THREAD_HEADER_H__
#define __POSIX_THREAD_HEADER_H__


#include <pthread.h>

class Thread {
  pthread_t thread_id_;
  void (*routine_)(void*);
  void* argument_;

  Thread(const Thread&);
  Thread& operator =(const Thread&);
public:
  explicit Thread(void (*routine)(void*) = NULL, void* argument = NULL)
    : thread_id_(0)
    , routine_(routine)
    , argument_(argument)
  {
  }

  ~Thread(void)
  {
    Join();
  }

  inline bool Start(void)
  {
    int ret = pthread_create(&thread_id_, NULL, &Thread::Routine, this);
    if (0 != ret)
      return false;

    return true;
  }

  inline void Join(void)
  {
    if (0 != thread_id_) {
      pthread_join(thread_id_, NULL);

      thread_id_ = 0;
    }
  }
private:
  static void* Routine(void* argument)
  {
    Thread* self = static_cast<Thread*>(argument);
    if (NULL == self)
      return NULL;

    if (NULL != self->routine_)
      self->routine_(self->argument_);

    return NULL;
  }
};


#endif  //! __POSIX_THREAD_HEADER_H__
