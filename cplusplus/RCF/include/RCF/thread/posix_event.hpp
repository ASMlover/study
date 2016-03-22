
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2013, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 2.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

//
// detail/posix_event.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_POSIX_EVENT_HPP
#define RCF_DETAIL_POSIX_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_HAS_PTHREADS)

#include <boost/assert.hpp>
#include <pthread.h>
#include <errno.h>

#include <RCF/thread/push_options.hpp>

#include <sys/time.h>

namespace RCF {
namespace detail {

class posix_event
  : private noncopyable
{
public:
  // Constructor.
  RCF_EXPORT posix_event();

  // Destructor.
  ~posix_event()
  {
    ::pthread_cond_destroy(&cond_);
  }

  // Signal one waiting thread.
  template <typename Lock>
  void signal_one(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
    ::pthread_cond_signal(&cond_); // Ignore EINVAL.
  }

  // Signal all waiting threads.
  template <typename Lock>
  void signal_all(Lock& lock)
  {
      BOOST_ASSERT(lock.locked());
      (void)lock;
      ::pthread_cond_broadcast(&cond_); // Ignore EINVAL.
  }

  // Reset the event.
  template <typename Lock>
  void clear(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
  }

  // Wait for the event to become signalled.
  template <typename Lock>
  void wait(Lock& lock)
  {
    // POSIX automatically unlocks and locks the mutex.

    BOOST_ASSERT(lock.locked());
    ::pthread_cond_wait(&cond_, &lock.mutex().mutex_); // Ignore EINVAL.
  }

  template <typename Lock>
  bool timed_wait(Lock& lock, boost::uint32_t waitMs)
  {
      BOOST_ASSERT(lock.locked());

      struct timeval tp = {0};
      gettimeofday(&tp, NULL);

      // Convert from timeval to timespec.
      struct timespec ts = {0};
      ts.tv_sec  = tp.tv_sec;
      ts.tv_nsec = tp.tv_usec * 1000;

      // Add waitMs to current time.
      ts.tv_sec += (waitMs / 1000);
      boost::uint32_t remainderMs = waitMs % 1000;
      ts.tv_nsec += (remainderMs * 1000 * 1000);

      // Check for overflow in tv_nsec.
      if (ts.tv_nsec >= 1000*1000*1000)
      {
          BOOST_ASSERT(ts.tv_nsec < 2*1000*1000*1000);
          ts.tv_sec += 1;
          ts.tv_nsec -= 1000*1000*1000;
      }

      // POSIX automatically unlocks and locks the mutex.

      int ret = ::pthread_cond_timedwait(&cond_, &lock.mutex().mutex_, &ts); // Ignore EINVAL.
      if (ret == ETIMEDOUT)
      {
        return false;
      }       

      return true;
  }

  // Signal the event.
  template <typename Lock>
  void notify_all(Lock& lock)
  {
      signal_all(lock);
  }

protected:
  ::pthread_cond_t cond_;
};

} // namespace detail
} // namespace RCF

#include <RCF/thread/pop_options.hpp>


#endif // defined(BOOST_HAS_PTHREADS)

#endif // RCF_DETAIL_POSIX_EVENT_HPP
