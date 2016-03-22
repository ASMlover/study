
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
// detail/thread.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef RCF_DETAIL_THREAD_HPP
#define RCF_DETAIL_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if 0
# include "null_thread.hpp"
#elif defined(BOOST_WINDOWS)
//# if defined(UNDER_CE)
//#  include "wince_thread.hpp"
//# else
#  include <RCF/thread/win_thread.hpp>
//# endif
#elif defined(BOOST_HAS_PTHREADS)
# include <RCF/thread/posix_thread.hpp>
#else
# error Only Windows and POSIX are supported!
#endif

namespace RCF {
namespace detail {

#if 0
typedef null_thread thread;
#elif defined(BOOST_WINDOWS)
//# if defined(UNDER_CE)
//typedef wince_thread thread;
//# else
typedef win_thread thread;
//# endif
#elif defined(BOOST_HAS_PTHREADS)
typedef posix_thread thread;
#endif

} // namespace detail
} // namespace RCF

#endif // RCF_DETAIL_THREAD_HPP
