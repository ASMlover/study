
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
// detail/impl/win_thread.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_IMPL_WIN_THREAD_IPP
#define RCF_DETAIL_IMPL_WIN_THREAD_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_WINDOWS) && !defined(UNDER_CE)

#include <process.h>
#include "RCF/thread/win_thread.hpp"
#include "RCF/thread/push_options.hpp"

namespace RCF {
namespace detail {

win_thread::~win_thread()
{
  ::CloseHandle(thread_);

  mArgPtr.reset();
}

void win_thread::join()
{
  HANDLE handles[2] = { mArgPtr->exit_event_, thread_ };
  ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
  ::WaitForSingleObject(thread_, INFINITE);
}

void win_thread::start_thread(unsigned int stack_size)
{
  mArgPtr->entry_event_ = ::CreateEvent(0, true, false, 0);
  if (!mArgPtr->entry_event_)
  {
    DWORD last_error = ::GetLastError();
    RCF_VERIFY(last_error == 0, Exception(_RcfError_ThreadingError("CreateEvent()"), last_error));
  }

  mArgPtr->exit_event_ = ::CreateEvent(0, true, false, 0);
  if (!mArgPtr->exit_event_)
  {
    DWORD last_error = ::GetLastError();
    RCF_VERIFY(last_error == 0, Exception(_RcfError_ThreadingError("CreateEvent()"), last_error));
  }

  unsigned int thread_id = 0;
  thread_ = reinterpret_cast<HANDLE>(::_beginthreadex(0,
        stack_size, win_thread_function, &mArgPtr, 0, &thread_id));
  if (!thread_)
  {
    DWORD last_error = ::GetLastError();
    RCF_VERIFY(last_error == 0, Exception(_RcfError_ThreadingError("_beginthreadex()"), last_error));
  }

  if (mArgPtr->entry_event_)
  {
	  ::WaitForSingleObject(mArgPtr->entry_event_, INFINITE);
  }
}

unsigned int __stdcall win_thread_function(void* arg)
{
  boost::shared_ptr<func_base> argPtr = * static_cast<boost::shared_ptr<func_base>*>(arg);
  ::SetEvent(argPtr->entry_event_);
  argPtr->run();
  ::SetEvent(argPtr->exit_event_);
  return 0;
}

} // namespace detail
} // namespace RCF

#include "RCF/thread/pop_options.hpp"

#endif // defined(BOOST_WINDOWS) && !defined(UNDER_CE)

#endif // RCF_DETAIL_IMPL_WIN_THREAD_IPP
