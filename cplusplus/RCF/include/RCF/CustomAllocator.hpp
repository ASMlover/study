
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

#ifndef INCLUDE_RCF_ALLOCATOR_HPP
#define INCLUDE_RCF_ALLOCATOR_HPP

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace RCF {

    void * RCF_new(std::size_t bytes);
    void RCF_delete(void * ptr);

} // namespace RCF

#endif // ! INCLUDE_RCF_ALLOCATOR_HPP
