
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
// detail/win_tss_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_WIN_TSS_PTR_HPP
#define RCF_DETAIL_WIN_TSS_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_WINDOWS)

#include <RCF/thread/push_options.hpp>

namespace RCF {
namespace detail {

// Helper function to create thread-specific storage.
RCF_EXPORT DWORD win_tss_ptr_create();

template <typename T>
class win_tss_ptr
  : private noncopyable
{
public:
  // Constructor.
  win_tss_ptr()
    : tss_key_(win_tss_ptr_create())
  {
  }

  // Destructor.
  ~win_tss_ptr()
  {
    ::TlsFree(tss_key_);
  }

  // Get the value.
  operator T*() const
  {
    return static_cast<T*>(::TlsGetValue(tss_key_));
  }

  // Set the value.
  void operator=(T* value)
  {
    ::TlsSetValue(tss_key_, value);
  }

private:
  // Thread-specific storage to allow unlocked access to determine whether a
  // thread is a member of the pool.
  DWORD tss_key_;
};

} // namespace detail
} // namespace RCF

#include <RCF/thread/pop_options.hpp>

#endif // defined(BOOST_WINDOWS)

#endif // RCF_DETAIL_WIN_TSS_PTR_HPP
