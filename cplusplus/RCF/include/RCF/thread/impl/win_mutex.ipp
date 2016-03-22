
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
// detail/impl/win_mutex.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_IMPL_WIN_MUTEX_IPP
#define RCF_DETAIL_IMPL_WIN_MUTEX_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_WINDOWS)

#include "RCF/thread/win_mutex.hpp"
#include "RCF/thread/push_options.hpp"

namespace RCF {
namespace detail {

win_mutex::win_mutex()
{
  commonCtor(false);
}

win_mutex::win_mutex(bool allowRecursiveLocking)
{
  commonCtor(allowRecursiveLocking);
}

void win_mutex::commonCtor(bool allowRecursiveLocking)
{
    int error = do_init();
    RCF_VERIFY(error == 0, Exception(_RcfError_ThreadingError("InitializeCriticalSectionAndSpinCount()"), error));

    RCF_UNUSED_VARIABLE(allowRecursiveLocking);

#ifndef NDEBUG
    mAllowRecursiveLocking = allowRecursiveLocking;
    mThreadId = 0;
#endif

}

int win_mutex::do_init()
{
#if defined(__MINGW32__)
  // Not sure if MinGW supports structured exception handling, so for now
  // we'll just call the Windows API and hope.
# if defined(UNDER_CE)
  ::InitializeCriticalSection(&crit_section_);
# else
  if (!::InitializeCriticalSectionAndSpinCount(&crit_section_, 0x80000000))
    return ::GetLastError();
# endif
  return 0;
#else
  __try
  {
# if defined(UNDER_CE)
    ::InitializeCriticalSection(&crit_section_);
# else
    if (!::InitializeCriticalSectionAndSpinCount(&crit_section_, 0x80000000))
      return ::GetLastError();
# endif
  }
  __except(GetExceptionCode() == STATUS_NO_MEMORY
      ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
  {
    return ERROR_OUTOFMEMORY;
  }

  return 0;
#endif
}

} // namespace detail
} // namespace RCF

#include "RCF/thread/pop_options.hpp"

#endif // defined(BOOST_WINDOWS)

#endif // RCF_DETAIL_IMPL_WIN_MUTEX_IPP
