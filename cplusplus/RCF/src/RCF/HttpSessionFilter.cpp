
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

#include <RCF/HttpSessionFilter.hpp>

#include <RCF/AsioServerTransport.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/HttpFrameFilter.hpp>
#include <RCF/ObjectPool.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/Tools.hpp>

#ifndef BOOST_WINDOWS
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

namespace RCF {

    HttpSession::HttpSession(const std::string & httpSessionId) :
        mRequestInProgress(false),
        mLastTouchMs(0),
        mHttpSessionId(httpSessionId),
        mHttpSessionIndex(0),
        mCachedReadBytesRequested(0)
    {
        RCF_LOG_3()(mHttpSessionId) << "HttpSession::HttpSession()";
    }

    HttpSession::~HttpSession()
    {
        RCF_LOG_3()(mHttpSessionId) << "HttpSession::~HttpSession()";
    }

    HttpSessionFilter::HttpSessionFilter(AsioNetworkSession& networkSession) :
        mNetworkSession(networkSession),
        mDummy()
    {
        resetState();
    }

    HttpSessionFilter::~HttpSessionFilter()
    {
    }

    void HttpSessionFilter::resetState()
    {
    }

    void HttpSessionFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        if ( mHttpSessionPtr && mWriteBuffers.size() > 0 )
        {
            // If we have a write outstanding, unhook the HTTP session, then write the HTTP response. 
            // When the write completes, another read will be triggered.

            mHttpSessionPtr->mCachedReadBytesRequested = 0;
            mHttpSessionPtr->mCachedReadBuffer = ByteBuffer();
            if ( bytesRequested )
            {
                mHttpSessionPtr->mCachedReadBytesRequested = bytesRequested;
                mHttpSessionPtr->mCachedReadBuffer = byteBuffer;
            }

            mNetworkSession.getTransportFilters(mHttpSessionPtr->mTransportFilters);
            mNetworkSession.setTransportFilters(mNoFilters);

            mNetworkSession.getAsioServerTransport().getServer().detachHttpSession(mHttpSessionPtr);
            mNetworkSession.mRcfSessionPtr.reset();

            mHttpSessionPtr->mRcfSessionPtr->mpNetworkSession = NULL;
            mHttpSessionPtr.reset();

            mpPostFilter->write(mWriteBuffers);
        }
        else if ( bytesRequested == 0 )
        {
            // Zero-byte read - pass through to network layer.

            mpPostFilter->read(byteBuffer, bytesRequested);
        }
        else if ( !mHttpSessionPtr )
        {
            // Don't have a HTTP frame yet. Issue a trivial read request on the network layer,
            // which will force the read of an entire HTTP frame.

            RCF_ASSERT(bytesRequested > 0);
            mReadBuffer = ByteBuffer(&mDummy, 1);
            mpPostFilter->read(mReadBuffer, 1);
        }
        else
        {
            // HTTP frame is available. Pass reads through to the network layer.
            if ( mReadBuffer.getLength() > 0 )
            {
                if ( byteBuffer.getLength() > 0 )
                {
                    std::size_t bytesToDeliver = RCF_MIN(mReadBuffer.getLength(), bytesRequested);
                    memcpy(byteBuffer.getPtr(), mReadBuffer.getPtr(), bytesToDeliver);
                    mReadBuffer = ByteBuffer(mReadBuffer, bytesToDeliver);
                    mpPreFilter->onReadCompleted(ByteBuffer(byteBuffer, 0, bytesToDeliver));
                }
                else
                {
                    std::size_t bytesToDeliver = RCF_MIN(mReadBuffer.getLength(), bytesRequested);
                    ByteBuffer buffer(mReadBuffer, 0, bytesToDeliver);
                    mReadBuffer = ByteBuffer(mReadBuffer, bytesToDeliver);
                    mpPreFilter->onReadCompleted(buffer);
                }
            }
            else
            {
                mpPostFilter->read(byteBuffer, bytesRequested);
            }
        }
    }

    void HttpSessionFilter::onReadCompleted(const ByteBuffer &byteBuffer)
    {
        if ( byteBuffer.getLength() == 0 )
        {
            mpPreFilter->onReadCompleted(byteBuffer);
        }
        else if ( !mHttpSessionPtr )
        {
            // HTTP frame is now available. Extract HTTP session ID,look up the HTTP 
            // session, install the session filters, then reissue the previously cached 
            // read operation.

            RCF_ASSERT(byteBuffer.getPtr() == mReadBuffer.getPtr());
            RCF_ASSERT(byteBuffer.getLength() == 1);
            HttpFrameFilter & httpFrame = static_cast<HttpFrameFilter &>(*mpPostFilter);
            const std::string & httpSessionId = httpFrame.getHttpSessionId();
            boost::uint32_t httpSessionIndex = httpFrame.getHttpSessionIndex();

            bool allowCreateHttpSession = false;
            if ( httpSessionIndex == 1 )
            {
                allowCreateHttpSession = true;
            }

            ExceptionPtr ePtr;

            mHttpSessionPtr = mNetworkSession.getAsioServerTransport().getServer().attachHttpSession(
                httpSessionId, 
                allowCreateHttpSession,
                ePtr);

            if ( !mHttpSessionPtr )
            {
                RCF_ASSERT(ePtr);
                httpFrame.onError(*ePtr);
                return;
            }

            RCF_ASSERT(mHttpSessionPtr);

            if ( mHttpSessionPtr->mHttpSessionIndex + 1 != httpSessionIndex )
            {
                Exception exc(Error(_RcfError_HttpRequestSessionIndex(
                    mHttpSessionPtr->mHttpSessionIndex + 1, 
                    httpSessionIndex)));

                httpFrame.onError(exc);
            }

            ++mHttpSessionPtr->mHttpSessionIndex;

            // Hook up the relevant filters and trigger a new sequence of reads.
            mNetworkSession.setTransportFilters(mHttpSessionPtr->mTransportFilters);
            RCF_ASSERT(mHttpSessionPtr->mRcfSessionPtr);
            mHttpSessionPtr->mRcfSessionPtr->setNetworkSession(mNetworkSession);
            mNetworkSession.mRcfSessionPtr = mHttpSessionPtr->mRcfSessionPtr;

            if ( mHttpSessionPtr->mCachedReadBytesRequested )
            {
                mHttpSessionPtr->mCachedReadBytesRequested = 0;
                mHttpSessionPtr->mCachedReadBuffer = ByteBuffer();
                
                CurrentRcfSessionSentry guard(mHttpSessionPtr->mRcfSessionPtr);
                mpPreFilter->onReadCompleted( ByteBuffer() );
            }
            else
            {
                mNetworkSession.onAppReadWriteCompleted(0);
            }
        }
        else
        {
            // Subsequent read completions - pass through to the HTTP session layer.
            mpPreFilter->onReadCompleted(byteBuffer);
        }
    }

    void HttpSessionFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        // If we write the response directly to network here, we ar at risk of a race
        // condition as the next request may be read in on another connection, before
        // the write completion on this connection has executed.

        // So instead we make a copy of the write buffers, and send a write completion
        // back up. The subsequent read operation will unhook the HTTP session and then 
        // write the response to the network.

        if ( mNetworkSession.mCloseAfterWrite )
        {
            // Special case, if this is the last message being written before the connection closes.
            mpPostFilter->write(byteBuffers);
        }
        else
        {
            std::size_t len = lengthByteBuffers(byteBuffers);
            ReallocBufferPtr bufferPtr = getObjectPool().getReallocBufferPtr();
            bufferPtr->resize(len);
            copyByteBuffers(byteBuffers, bufferPtr->getPtr());
            mWriteBuffers.resize(0);
            mWriteBuffers.push_back(ByteBuffer(bufferPtr));

            mpPreFilter->onWriteCompleted(len);
        }
    }

    void HttpSessionFilter::onWriteCompleted(std::size_t bytesTransferred)
    {
        // Issue zero-byte read back down to the network layer.

        // Assuming here that the HttpFrameFilter below this filter has written the entire
        // frame, before reporting a write completion.

        RCF_UNUSED_VARIABLE(bytesTransferred);
        mWriteBuffers.resize(0);

        HttpFrameFilter& httpFrame = static_cast<HttpFrameFilter&>(*mpPostFilter);
        const std::string & connectionHeader = httpFrame.getConnectionHeader();
        if ( 0 != stricmp(connectionHeader.c_str(), "close") )
        {
            mpPostFilter->read(ByteBuffer(), 0);
        }
    }

    int HttpSessionFilter::getFilterId() const
    {
        return RcfFilter_Unknown;
    }

} // namespace RCF
