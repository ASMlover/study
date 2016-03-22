
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

#include <RCF/HttpConnectFilter.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace RCF {

    HttpConnectFilter::HttpConnectFilter() :
        mServerAddr(),
        mServerPort(0)
    {
        resetState();
    }

    HttpConnectFilter::HttpConnectFilter(const std::string serverAddr, int serverPort) :
        mServerAddr(serverAddr),
        mServerPort(serverPort)
    {
        resetState();
    }

    void HttpConnectFilter::resetState()
    {
        mPassThrough = false;
        mOrigWriteBuffers.clear();
        mHttpConnectRequest.clear();
        mHttpConnectResponse.clear();
        mWritePos = 0;
        mReadPos = 0;
        mReadVector.clear();
    }

    void HttpConnectFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        RCF_ASSERT(mPassThrough);
        mpPostFilter->read(byteBuffer, bytesRequested);
    }

    void HttpConnectFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        if (mPassThrough)
        {
            mpPostFilter->write(byteBuffers);
        }
        else
        {
            ClientStub * pClientStub = getTlsClientStubPtr();
            RCF_ASSERT(pClientStub);
            bool usingProxy = pClientStub->getHttpProxy().size() > 0;
            if (!usingProxy)
            {
                mPassThrough = true;
                write(byteBuffers);
            }
            else
            {
                mOrigWriteBuffers = byteBuffers;

                MemOstream os;
                os 
                    << "CONNECT " << mServerAddr << ":" << mServerPort << " HTTP/1.1\r\n"
                    << "Host: " << mServerAddr << ":" << mServerPort << "\r\n"
                    << "Proxy-Connection: Keep-Alive\r\n"
                    << "\r\n";

                mHttpConnectRequest = os.string();

                mWritePos = 0;

                std::vector<ByteBuffer> writeBuffers;

                writeBuffers.push_back( ByteBuffer(
                    (char * ) mHttpConnectRequest.c_str(), 
                    mHttpConnectRequest.size()));

                mpPostFilter->write(writeBuffers);
            }
        }
    }

    void HttpConnectFilter::onReadCompleted(const ByteBuffer &byteBuffer)
    {
        if (mPassThrough)
        {
            mpPreFilter->onReadCompleted(byteBuffer);
        }
        else
        {
            RCF_ASSERT(byteBuffer.getLength() <= mReadVector.size());
            mReadPos += byteBuffer.getLength();

            mHttpConnectResponse.assign(&mReadVector[0], mReadPos);
            std::size_t endPos = mHttpConnectResponse.find("\r\n\r\n");
            if (endPos != std::string::npos)
            {
                std::string http;
                std::string httpStatus;
                std::string firstLine = mHttpConnectResponse.substr(0, mHttpConnectResponse.find("\r\n"));
                MemIstream is(firstLine.c_str(), firstLine.size());
                is >> http >> httpStatus;
                boost::trim_left(httpStatus);
                if (boost::istarts_with(firstLine, "HTTP/") && boost::istarts_with(httpStatus, "200"))
                {
                    // CONNECT is OK. 
                    mPassThrough = true;
                    mpPostFilter->write(mOrigWriteBuffers);
                }
                else
                {
                    // Something went wrong.
                    RCF_THROW( Exception(_RcfError_HttpConnectFailed(firstLine, mHttpConnectResponse)) );
                }
            }
            else
            {
                // Read some more.

                if (mReadPos == mReadVector.size())
                {
                    RCF_THROW( Exception("Invalid HTTP CONNECT response.") );
                }

                ByteBuffer buffer(&mReadVector[mReadPos], mReadVector.size() - mReadPos);
                mpPostFilter->read(buffer, buffer.getLength());
            }
        }
    }

    void HttpConnectFilter::onWriteCompleted(std::size_t bytesTransferred)
    {
        if (mPassThrough)
        {
            mpPreFilter->onWriteCompleted(bytesTransferred);
        }
        else
        {
            mWritePos += bytesTransferred;
            if (mWritePos < mHttpConnectRequest.size())
            {
                std::vector<ByteBuffer> writeBuffers;

                writeBuffers.push_back( ByteBuffer(
                    (char *) (mHttpConnectRequest.c_str() + mWritePos), 
                    mHttpConnectRequest.size() - mWritePos));

                mpPostFilter->write(writeBuffers);
            }
            else
            {
                // CONNECT request has been sent, now wait for reply.
                mReadVector.resize(1024);
                ByteBuffer buffer(&mReadVector[0], mReadVector.size());
                mpPostFilter->read(buffer, buffer.getLength());
            }
        }
    }

    int HttpConnectFilter::getFilterId() const
    {
        return RcfFilter_Unknown;
    }

} // namespace RCF
