
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

#ifndef INCLUDE_RCF_HTTPFRAMEFILTER_HPP
#define INCLUDE_RCF_HTTPFRAMEFILTER_HPP

#include <map>

#include <boost/shared_ptr.hpp>

#include <RCF/Filter.hpp>
#include <RCF/ByteBuffer.hpp>

namespace RCF {

    class Exception;

    class HttpFrameFilter : public Filter
    {
    public:

        // Server-side constructor.
        HttpFrameFilter(std::size_t maxMessageLength);

        // Client-side constructor.
        HttpFrameFilter(
            const std::string       serverAddr, 
            int                     serverPort);
        
        ~HttpFrameFilter();

        void                    init();
        void                    resetState();

        void                    read(
                                    const ByteBuffer &      byteBuffer,
                                    std::size_t             bytesRequested);

        void                    write(const std::vector<ByteBuffer> &byteBuffers);

        void                    onReadCompleted(const ByteBuffer &byteBuffer);

        void                    onWriteCompleted(std::size_t bytesTransferred);

        int                     getFilterId() const;

        virtual std::size_t     getFrameSize();

        const std::string &     getHttpSessionId();
        boost::uint32_t         getHttpSessionIndex();
        const std::string &     getConnectionHeader();

        void                    onError(const Exception& e);

        // If these are set, then we are doing a HTTP response with chunked transfer encoding.
        bool                    mChunkedResponseMode;
        std::size_t             mChunkedResponseCounter;

        void                    tryParseHttpHeader();
        void                    tryParseHttpChunkHeader();



    private:

        void                    sendServerError(int error);

        void                    resizeReadBuffer(std::size_t newSize);

        std::string                             mServerAddr;
        int                                     mServerPort;

        bool                                    mClientSide;
        std::string                             mHttpSessionId;
        boost::uint32_t                         mHttpSessionIndex;
        std::string                             mConnectionHeader;
        std::string                             mTransferEncoding;

        MemOstreamPtr                           mOsPtr;
        std::vector<ByteBuffer>                 mWriteBuffers;
        std::size_t                             mWritePos;

        ByteBuffer                              mOrigReadBuffer;
        std::size_t                             mOrigBytesRequested;

        ReallocBufferPtr                        mReadBufferPtr;
        std::size_t                             mBytesReceived;
        std::size_t                             mReadPos;

        bool                                    mProtocolChecked;
        std::size_t                             mHeaderLen;
        std::size_t                             mContentLen;
        std::size_t                             mChunkHeaderLen;
        std::size_t                             mChunkLen;
        std::size_t                             mFrameLen;
        std::size_t                             mMaxReadPos;
        std::size_t                             mMaxMessageLength;

        std::string                             mHttpMessageHeader;
        std::vector<std::string>                mHeaderLines;
        std::string                             mPrevHttpSessionId;
        boost::uint32_t                         mPrevHttpSessionIndex;

        std::string                             mRequestLine;
        std::string                             mResponseLine;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_HTTPFRAMEFILTER_HPP
