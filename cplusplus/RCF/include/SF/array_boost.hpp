
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

#ifndef INCLUDE_SF_ARRAY_BOOST_HPP
#define INCLUDE_SF_ARRAY_BOOST_HPP

#include <SF/SerializeArray.hpp>

#include <boost/array.hpp>

namespace SF {

    class Archive;

    template<typename T, std::size_t N>
    void serialize_vc6(SF::Archive & ar, boost::array<T, N> & a, const unsigned int)
    {
        serialize_array_impl(ar, a);
    }

} // namespace SF

#endif // ! INCLUDE_SF_ARRAY_BOOST_HPP
