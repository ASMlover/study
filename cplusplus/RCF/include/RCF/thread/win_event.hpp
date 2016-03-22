
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
// detail/win_event.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_WIN_EVENT_HPP
#define RCF_DETAIL_WIN_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_WINDOWS)

#include <boost/assert.hpp>

#include <RCF/thread/push_options.hpp>

#include <RCF/thread/mutex.hpp>

namespace RCF {
namespace detail {

class RCF_EXPORT win_event
  : private noncopyable
{
public:
  // Constructor.
  win_event();

  // Destructor.
  ~win_event()
  {
    ::CloseHandle(event_);
  }

  // Signal the event.
  template <typename Lock>
  void signal_all(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
    if (mWaitCount > 0)
    {
        ::SetEvent(event_);
    }
  }

  // Reset the event.
  template <typename Lock>
  void clear(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());
    (void)lock;
    ::ResetEvent(event_);
  }

  // Wait for the event to become signalled.
  template <typename Lock>
  void wait(Lock& lock)
  {
    BOOST_ASSERT(lock.locked());    
    lock.unlock();

    {
        mutex::scoped_lock waitLock(mWaitCountMutex);
        ++mWaitCount;
    }

    ::WaitForSingleObject(event_, INFINITE);

    {
        mutex::scoped_lock waitLock(mWaitCountMutex);
        --mWaitCount;
        if (mWaitCount == 0)
        {
            // Set signalled state back to false.
            ::ResetEvent(event_);
        }
    }
    
    lock.lock();
    
  }

  // Wait for the event to become signalled.
  template <typename Lock>
  bool timed_wait(Lock& lock, boost::uint32_t waitMs)
  {
      BOOST_ASSERT(lock.locked());
      
      lock.unlock();

      {
          mutex::scoped_lock waitLock(mWaitCountMutex);
          ++mWaitCount;
      }

      DWORD ret = ::WaitForSingleObject(event_, waitMs);
      assert(ret != WAIT_ABANDONED && ret != WAIT_FAILED);

      {
          mutex::scoped_lock waitLock(mWaitCountMutex);
          --mWaitCount;
          if (ret != WAIT_TIMEOUT && mWaitCount == 0)
          {
              // Set signalled state back to false.
              ::ResetEvent(event_);
          }
      }

      lock.lock();
      if (ret == WAIT_TIMEOUT)
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

  // Newer versions of Windows have built in condition variables (InitializeConditionVariable() etc)
  // but they are not supported in XP, Server 2003, and older. So for now we use
  // events instead, with the mWatiCount workaround below for resetting the event.
  HANDLE event_;

  // Number of threads waiting on the event. When the threads are signalled, 
  // the counter is decremented, and the thread that decrements the counter
  // to zero will reset the event. The event is a "manual reset" event, as
  // an "auto reset" event only releases 1 thread at a time.
  mutex mWaitCountMutex;
  int mWaitCount;
};

} // namespace detail
} // namespace RCF

#include <RCF/thread/pop_options.hpp>

#endif // defined(BOOST_WINDOWS)

#endif // RCF_DETAIL_WIN_EVENT_HPP
