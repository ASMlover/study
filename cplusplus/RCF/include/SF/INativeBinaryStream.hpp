
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

#ifndef INCLUDE_SF_INATIVEBINARYSTREAM_HPP
#define INCLUDE_SF_INATIVEBINARYSTREAM_HPP

#include <SF/Stream.hpp>

namespace SF {

    class INativeBinaryStream : public IStream
    {
    public:
        INativeBinaryStream(RCF::MemIstream &is) : IStream(is)
        {}

        INativeBinaryStream(std::istream &is) : IStream(is)
        {}

        I_Encoding &getEncoding()
        {
            return mEncoding;
        }

    private:
        EncodingBinaryNative mEncoding;
    };

}

#endif // ! INCLUDE_SF_INATIVEBINARYSTREAM_HPP
