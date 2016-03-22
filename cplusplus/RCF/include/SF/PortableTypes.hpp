
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

#ifndef INCLUDE_SF_PORTABLETYPES_HPP
#define INCLUDE_SF_PORTABLETYPES_HPP

#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

namespace SF {

    typedef char                                Byte8;
    typedef boost::uint32_t                     UInt32;

    BOOST_STATIC_ASSERT( sizeof(Byte8) == 1 );
    BOOST_STATIC_ASSERT( sizeof(UInt32) == 4 );

} // namespace SF

#endif // ! INCLUDE_SF_PORTABLETYPES_HPP
