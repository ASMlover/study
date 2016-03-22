
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

#ifndef INCLUDE_SF_OBINARYSTREAM_HPP
#define INCLUDE_SF_OBINARYSTREAM_HPP

#include <SF/Stream.hpp>

namespace SF {

    class OBinaryStream : public OStream
    {
    public:
        OBinaryStream() : OStream()
        {}

        OBinaryStream(RCF::MemOstream &os) : OStream(os)
        {}

        OBinaryStream(std::ostream &os) : OStream(os)
        {}

        I_Encoding &getEncoding()
        {
            return mEncoding;
        }

    private:
        EncodingBinaryPortable mEncoding;
    };

}

#endif // ! INCLUDE_SF_OBINARYSTREAM_HPP
