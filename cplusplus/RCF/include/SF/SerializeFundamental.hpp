
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

#ifndef INCLUDE_SF_SERIALIZEFUNDAMENTAL_HPP
#define INCLUDE_SF_SERIALIZEFUNDAMENTAL_HPP

#include <SF/Archive.hpp>
#include <SF/DataPtr.hpp>
#include <SF/I_Stream.hpp>
#include <SF/Stream.hpp>
#include <SF/Tools.hpp>

namespace SF {

    // serialize fundamental types

    template<typename T>
    inline void serializeFundamental(
        SF::Archive &ar, 
        T &t,
        unsigned int count = 1)
    {
        typedef typename RCF::RemoveCv<T>::type U;
        BOOST_STATIC_ASSERT( RCF::IsFundamental<U>::value );
        U * pt = const_cast<U *>(&t);

        if (ar.isRead())
        {
            I_Encoding &encoding = ar.getIstream()->getEncoding();
            DataPtr data;
            ar.getIstream()->get(data);
            if (count > 1 && count != encoding.getCount(data, pt) )
            {
                // static array size mismatch
                RCF::Exception e(RCF::_SfError_DataFormat());
                RCF_THROW(e)(typeid(U).name())(count)(encoding.getCount(data, pt));
            }
            encoding.toObject(data, pt, count);
        }
        else if (ar.isWrite())
        {
            I_Encoding &encoding = ar.getOstream()->getEncoding();
            DataPtr data;
            encoding.toData(data, pt, count );
            ar.getOstream()->put(data);
        }
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZEFUNDAMENTAL_HPP
