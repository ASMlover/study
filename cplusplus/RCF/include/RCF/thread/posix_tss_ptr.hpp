
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
// detail/posix_tss_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_POSIX_TSS_PTR_HPP
#define RCF_DETAIL_POSIX_TSS_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(BOOST_HAS_PTHREADS)

#include <pthread.h>

#include <RCF/thread/push_options.hpp>

namespace RCF {
namespace detail {

// Helper function to create thread-specific storage.
RCF_EXPORT void posix_tss_ptr_create(pthread_key_t& key);

template <typename T>
class posix_tss_ptr
  : private noncopyable
{
public:
  // Constructor.
  posix_tss_ptr()
  {
    posix_tss_ptr_create(tss_key_);
  }

  // Destructor.
  ~posix_tss_ptr()
  {
    ::pthread_key_delete(tss_key_);
  }

  // Get the value.
  operator T*() const
  {
    return static_cast<T*>(::pthread_getspecific(tss_key_));
  }

  // Set the value.
  void operator=(T* value)
  {
    ::pthread_setspecific(tss_key_, value);
  }

private:
  // Thread-specific storage to allow unlocked access to determine whether a
  // thread is a member of the pool.
  pthread_key_t tss_key_;

};

} // namespace detail
} // namespace RCF

#include <RCF/thread/pop_options.hpp>

#endif // defined(BOOST_HAS_PTHREADS)

#endif // RCF_DETAIL_POSIX_TSS_PTR_HPP
