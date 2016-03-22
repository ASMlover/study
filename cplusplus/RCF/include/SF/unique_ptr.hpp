
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

#ifndef INCLUDE_SF_UNIQUE_PTR_HPP
#define INCLUDE_SF_UNIQUE_PTR_HPP

#include <memory>

#include <SF/SerializeSmartPtr.hpp>

namespace SF {

    // serialize std::unique_ptr
    SF_SERIALIZE_SIMPLE_SMARTPTR( std::unique_ptr );

} // namespace SF

#endif // ! INCLUDE_SF_UNIQUE_PTR_HPP
