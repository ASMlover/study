
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

#ifndef INCLUDE_SF_SERIALIZEARRAY_HPP
#define INCLUDE_SF_SERIALIZEARRAY_HPP

#include <cstddef>

#include <RCF/Exception.hpp>
#include <SF/Archive.hpp>

namespace SF {

    class Archive;

    // Serialization for boost::array<>, std::array<>, etc.

    template<typename ArrayType>
    void serialize_array_impl(SF::Archive & ar, ArrayType & a)
    {
        if (ar.isRead())
        {
            unsigned int count = 0;
            ar & count;

            RCF_VERIFY(
                static_cast<std::size_t>(count) == a.size(), 
                RCF::Exception(RCF::_RcfError_RcfError_ArraySizeMismatch( 
                    static_cast<unsigned int>(a.size()), 
                    count)));

            for (std::size_t i=0; i<a.size(); ++i)
            {
                ar & a[i];
            }
        }
        else if (ar.isWrite())
        {
            unsigned int count = static_cast<unsigned int>(a.size());
            ar & count;

            for (std::size_t i=0; i<a.size(); ++i)
            {
                ar & a[i];
            }
        }
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZEARRAY_HPP
