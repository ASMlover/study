
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

#ifndef INCLUDE_RCF_XORFILTER_HPP
#define INCLUDE_RCF_XORFILTER_HPP

#include <vector>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Filter.hpp>

namespace RCF {

    class XorFilter : public IdentityFilter
    {
    public:
        int getFilterId() const;

        XorFilter();
        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested);
        void write(const std::vector<ByteBuffer> &byteBuffers);
        void onReadCompleted(const ByteBuffer &byteBuffer);
        void onWriteCompleted(std::size_t bytesTransferred);

    private:
        std::size_t mTotalBytes;
        std::vector<ByteBuffer> mByteBuffers;
        std::vector<ByteBuffer> mTempByteBuffers;
        char mMask;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_XORFILTER_HPP
