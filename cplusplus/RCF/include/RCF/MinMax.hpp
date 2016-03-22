
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

#ifndef INCLUDE_RCF_MINMAX_HPP
#define INCLUDE_RCF_MINMAX_HPP

// Macros in Windows platform headers tend to make it difficult to use
// std::min/std::max.

#include <algorithm>

#define RCF_MIN (std::min)
#define RCF_MAX (std::max)

#endif // ! INCLUDE_RCF_MINMAX_HPP
