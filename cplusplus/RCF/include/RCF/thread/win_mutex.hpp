
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
// detail/win_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_WIN_MUTEX_HPP
#define RCF_DETAIL_WIN_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_WINDOWS)

#include <RCF/thread/scoped_lock.hpp>

#include <RCF/thread/push_options.hpp>

namespace RCF {
namespace detail {

class RCF_EXPORT win_mutex
  : private noncopyable
{
public:
  typedef RCF::detail::scoped_lock<win_mutex> scoped_lock;

  // Constructor.
  win_mutex();
  win_mutex(bool allowRecursiveLocking);  

  // Destructor.
  ~win_mutex()
  {
    ::DeleteCriticalSection(&crit_section_);
  }

  // Lock the mutex.
  void lock()
  {

#ifndef NDEBUG
    int threadId = GetCurrentThreadId();
    if (!mAllowRecursiveLocking)
    {
        assert(threadId != mThreadId && "Recursive locking detected.");
    }
#endif

    ::EnterCriticalSection(&crit_section_);

#ifndef NDEBUG
    mThreadId = threadId;
#endif

  }

  // Unlock the mutex.
  void unlock()
  {
#ifndef NDEBUG
     mThreadId = 0;
#endif

    ::LeaveCriticalSection(&crit_section_);
  }

private:

  void commonCtor(bool allowRecursiveLocking);

  // Initialisation must be performed in a separate function to the constructor
  // since the compiler does not support the use of structured exceptions and
  // C++ exceptions in the same function.
  int do_init();

  ::CRITICAL_SECTION crit_section_;

#ifndef NDEBUG
  // Debug tool to detect recursive locking.
  int mThreadId;
  bool mAllowRecursiveLocking;
#endif
  

};

} // namespace detail
} // namespace RCF

#include <RCF/thread/pop_options.hpp>

#endif // defined(BOOST_WINDOWS)

#endif // RCF_DETAIL_WIN_MUTEX_HPP
