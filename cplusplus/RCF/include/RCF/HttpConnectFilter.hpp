
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

#ifndef INCLUDE_RCF_HTTPCONNECTFILTER_HPP
#define INCLUDE_RCF_HTTPCONNECTFILTER_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/Filter.hpp>
#include <RCF/ByteBuffer.hpp>

namespace RCF {

    class HttpConnectFilter : public Filter
    {
    public:

        HttpConnectFilter();
        HttpConnectFilter(const std::string serverAddr, int serverPort);

        void resetState();

        void read(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested);

        void write(const std::vector<ByteBuffer> &byteBuffers);

        void onReadCompleted(const ByteBuffer &byteBuffer);

        void onWriteCompleted(std::size_t bytesTransferred);

        int getFilterId() const;

    private:

        std::string mServerAddr;
        int mServerPort;

        bool mPassThrough;

        std::vector<ByteBuffer> mOrigWriteBuffers;

        std::string mHttpConnectRequest;
        std::string mHttpConnectResponse;

        std::size_t mWritePos;
        std::size_t mReadPos;

        std::vector<char> mReadVector;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_HTTPCONNECTFILTER_HPP
