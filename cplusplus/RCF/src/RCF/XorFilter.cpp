
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

#include <RCF/XorFilter.hpp>

namespace RCF {

    void createNonReadOnlyByteBuffers(
        std::vector<ByteBuffer> &nonReadOnlyByteBuffers,
        const std::vector<ByteBuffer> &byteBuffers)
    {
        nonReadOnlyByteBuffers.resize(0);
        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            if (byteBuffers[i].getLength()  > 0)
            {
                if (byteBuffers[i].getReadOnly())
                {
                    boost::shared_ptr< std::vector<char> > spvc(
                        new std::vector<char>( byteBuffers[i].getLength()));

                    memcpy(
                        &(*spvc)[0],
                        byteBuffers[i].getPtr(),
                        byteBuffers[i].getLength() );

                    nonReadOnlyByteBuffers.push_back(
                        ByteBuffer(&(*spvc)[0], spvc->size(), spvc));
                }
                else
                {
                    nonReadOnlyByteBuffers.push_back(byteBuffers[i]);
                }
            }
        }
    }

    XorFilter::XorFilter() :
        mMask('U')
    {}

    void XorFilter::read(const ByteBuffer &byteBuffer, std::size_t bytesRequested)
    {
        getPostFilter().read(byteBuffer, bytesRequested);
    }

    void XorFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        // need to make copies of any readonly buffers before transforming
        // TODO: only do this if at least one byte buffer is non-readonly

        mTotalBytes = lengthByteBuffers(byteBuffers);

        createNonReadOnlyByteBuffers(mByteBuffers, byteBuffers);

        // in place transformation
        for (std::size_t i=0; i<mByteBuffers.size(); ++i)
        {
            for (std::size_t j=0; j<mByteBuffers[i].getLength() ; ++j)
            {
                mByteBuffers[i].getPtr() [j] ^= mMask;
            }
        }

        getPostFilter().write(mByteBuffers);
    }

    void XorFilter::onReadCompleted(const ByteBuffer &byteBuffer)
    {
        for (std::size_t i=0; i<byteBuffer.getLength() ; ++i)
        {
            byteBuffer.getPtr() [i] ^= mMask;
        }
        getPreFilter().onReadCompleted(byteBuffer);
    }

    void XorFilter::onWriteCompleted(std::size_t bytesTransferred)
    {
        if (bytesTransferred == lengthByteBuffers(mByteBuffers))
        {
            mByteBuffers.resize(0);
            mTempByteBuffers.resize(0);
            getPreFilter().onWriteCompleted(mTotalBytes);
        }
        else
        {
            sliceByteBuffers(mTempByteBuffers, mByteBuffers, bytesTransferred);
            mByteBuffers = mTempByteBuffers;
            getPostFilter().write(mByteBuffers);
        }
    }

    int XorFilter::getFilterId() const
    {
        return RcfFilter_Xor;
    }

} // namespace RCF
