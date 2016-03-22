
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

#ifndef INCLUDE_UTIL_PLATFORM_OS_OUTPUTDEBUGSTRING_HPP
#define INCLUDE_UTIL_PLATFORM_OS_OUTPUTDEBUGSTRING_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS) || defined(_WIN32)
#include "Windows/OutputDebugString.hpp"
#elif defined(__CYGWIN__)
#include "Unix/OutputDebugString.hpp"
#elif defined(__unix__)
#include "Unix/OutputDebugString.hpp"
#elif defined(__APPLE__)
#include "Unix/OutputDebugString.hpp"
#else
#include "UnknownOS/OutputDebugString.hpp"
#endif

#endif // ! INCLUDE_UTIL_PLATFORM_OS_OUTPUTDEBUGSTRING_HPP
