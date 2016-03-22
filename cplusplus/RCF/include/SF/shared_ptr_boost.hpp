
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

#ifndef INCLUDE_SF_SHARED_PTR_BOOST_HPP
#define INCLUDE_SF_SHARED_PTR_BOOST_HPP

#include <SF/SerializeSmartPtr.hpp>

#include <boost/shared_ptr.hpp>

namespace SF {

    SF_SERIALIZE_REFCOUNTED_SMARTPTR( boost::shared_ptr );

} // namespace SF

#endif // ! INCLUDE_SF_SHARED_PTR_BOOST_HPP
