
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

#include <RCF/AsioServerTransport.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Asio.hpp>
#include <RCF/Filter.hpp>
#include <RCF/ConnectedClientTransport.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/HttpFrameFilter.hpp>
#include <RCF/HttpSessionFilter.hpp>
#include <RCF/MethodInvocation.hpp>
#include <RCF/ObjectPool.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TimedBsdSockets.hpp>

namespace RCF {    

    // FilterAdapter

    class FilterAdapter : public RCF::IdentityFilter
    {
    public:
        FilterAdapter(AsioNetworkSession &networkSession) :
            mNetworkSession(networkSession)
        {}

    private:
        void read(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested)
        {
            mNetworkSession.read(byteBuffer, bytesRequested);
        }

        void write(
            const std::vector<ByteBuffer> &byteBuffers)
        {
            mNetworkSession.write(byteBuffers);
        }

        void onReadCompleted(
            const ByteBuffer &byteBuffer)
        {
            if ( mNetworkSession.mAppReadBufferPtr )
            {
                char * pchOriginal = mNetworkSession.mAppReadBufferPtr->getPtr() + mNetworkSession.mAppReadBufferPtr->size() - mNetworkSession.mReadBufferRemaining;
                if ( byteBuffer.getLength() > 0 && byteBuffer.getPtr() != pchOriginal )
                {
                    memcpy(pchOriginal, byteBuffer.getPtr(), byteBuffer.getLength());
                }
            }
            mNetworkSession.onAppReadWriteCompleted(byteBuffer.getLength());
        }

        void onWriteCompleted(
            std::size_t bytesTransferred)
        {
            mNetworkSession.onAppReadWriteCompleted(bytesTransferred);
        }

        int getFilterId() const
        {
            return RcfFilter_Unknown;
        }

        AsioNetworkSession &mNetworkSession;
    };


    ReadHandler::ReadHandler(AsioNetworkSessionPtr networkSessionPtr) : 
        mNetworkSessionPtr(networkSessionPtr)
    {
    }

    void ReadHandler::operator()(AsioErrorCode err, std::size_t bytes)
    {
        mNetworkSessionPtr->onNetworkReadCompleted(err, bytes);
    }

    void * ReadHandler::allocate(std::size_t size)
    {
        if (mNetworkSessionPtr->mReadHandlerBuffer.size() < size)
        {
            mNetworkSessionPtr->mReadHandlerBuffer.resize(size);
        }
        return & mNetworkSessionPtr->mReadHandlerBuffer[0];
    }

    WriteHandler::WriteHandler(AsioNetworkSessionPtr networkSessionPtr) : 
        mNetworkSessionPtr(networkSessionPtr)
    {
    }

    void WriteHandler::operator()(AsioErrorCode err, std::size_t bytes)
    {
        mNetworkSessionPtr->onNetworkWriteCompleted(err, bytes);
    }

    void * WriteHandler::allocate(std::size_t size)
    {
        if (mNetworkSessionPtr->mWriteHandlerBuffer.size() < size)
        {
            mNetworkSessionPtr->mWriteHandlerBuffer.resize(size);
        }
        return & mNetworkSessionPtr->mWriteHandlerBuffer[0];
    }

    void * asio_handler_allocate(std::size_t size, ReadHandler * pHandler)
    {
        return pHandler->allocate(size);
    }

    void asio_handler_deallocate(void * pointer, std::size_t size, ReadHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pointer);
        RCF_UNUSED_VARIABLE(size);
        RCF_UNUSED_VARIABLE(pHandler);
    }

    void * asio_handler_allocate(std::size_t size, WriteHandler * pHandler)
    {
        return pHandler->allocate(size);
    }

    void asio_handler_deallocate(void * pointer, std::size_t size, WriteHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pointer);
        RCF_UNUSED_VARIABLE(size);
        RCF_UNUSED_VARIABLE(pHandler);
    }

    void AsioNetworkSession::postRead()
    {
        if (mLastError)
        {
            return;
        }

        mState = AsioNetworkSession::ReadingDataCount;

        mAppReadByteBuffer.clear();
        mAppReadBufferPtr.reset();

        mNetworkReadByteBuffer.clear();
        mNetworkReadBufferPtr.reset();

        mReadBufferRemaining = 0;
        mIssueZeroByteRead = true;
        beginRead();
    }

    void AsioNetworkSession::postWrite(
        std::vector<ByteBuffer> &byteBuffers)
    {
        if (mLastError)
        {
            return;
        }

        BOOST_STATIC_ASSERT(sizeof(unsigned int) == 4);

        mSlicedWriteByteBuffers.resize(0);
        mWriteByteBuffers.resize(0);

        std::copy(
            byteBuffers.begin(),
            byteBuffers.end(),
            std::back_inserter(mWriteByteBuffers));

        byteBuffers.resize(0);

        if (!mTransport.mCustomFraming)
        {
            // Add frame (4 byte length prefix).
            int messageSize = 
                static_cast<int>(RCF::lengthByteBuffers(mWriteByteBuffers));

            ByteBuffer &byteBuffer = mWriteByteBuffers.front();

            RCF_ASSERT_GTEQ(byteBuffer.getLeftMargin() , 4);
            byteBuffer.expandIntoLeftMargin(4);
            memcpy(byteBuffer.getPtr(), &messageSize, 4);
            RCF::machineToNetworkOrder(byteBuffer.getPtr(), 4, 1);
        }

        mState = AsioNetworkSession::WritingData;
        
        mWriteBufferRemaining = RCF::lengthByteBuffers(mWriteByteBuffers);
        
        beginWrite();
    }

    void AsioNetworkSession::postClose()
    {
        close();
    }

    ServerTransport & AsioNetworkSession::getServerTransport()
    {
        return mTransport;
    }

    AsioServerTransport & AsioNetworkSession::getAsioServerTransport()
    {
        return mTransport;
    }

    const RemoteAddress &
        AsioNetworkSession::getRemoteAddress()
    {
        return implGetRemoteAddress();
    }

    bool AsioNetworkSession::isConnected()
    {
        return implIsConnected();
    }

    // NetworkSession

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
#endif

    AsioNetworkSession::AsioNetworkSession(
        AsioServerTransport & transport,
        AsioIoService & ioService) :
            mIoService(ioService),
            mState(Ready),
            mIssueZeroByteRead(false),
            mReadBufferRemaining(),
            mWriteBufferRemaining(),
            mTransport(transport),
            mFilterAdapterPtr(new FilterAdapter(*this)),
            mCloseAfterWrite()
    {
        std::vector<FilterPtr> wireFilters;

        if (transport.mWireProtocol == Wp_Http || transport.mWireProtocol == Wp_Https)
        {
#if RCF_FEATURE_HTTP==1
            wireFilters.push_back(FilterPtr(new HttpSessionFilter(*this)));
            wireFilters.push_back( FilterPtr(new HttpFrameFilter(transport.getMaxMessageLength())) );
#else
            RCF_ASSERT(0 && "This RCF build does not support HTTP tunneling.");
#endif
        }

        if (transport.mWireProtocol == Wp_Https)
        {
            FilterPtr sslFilterPtr;
            if (transport.mpServer->getSslImplementation() == Si_Schannel)
            {
                sslFilterPtr = transport.mpServer->createFilter(RcfFilter_SspiSchannel);
            }
            else
            {
                sslFilterPtr = transport.mpServer->createFilter(RcfFilter_OpenSsl);
            }
            if (!sslFilterPtr)
            {
                RCF_THROW( Exception(_RcfError_SslNotSupported()) );
            }

            wireFilters.push_back( sslFilterPtr );
        }

        if (wireFilters.size() > 0)
        {
            setWireFilters(wireFilters);
        }
    }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    AsioNetworkSession::~AsioNetworkSession()
    {
        RCF_DTOR_BEGIN

        // TODO: invoke accept if appropriate
        // TODO: need a proper acceptex strategy in the first place
        //RCF_ASSERT(mState != Accepting);

        mTransport.unregisterSession(mWeakThisPtr);

        RCF_LOG_4()(mState)(mRcfSessionPtr.get()) << "AsioNetworkSession - destructor.";

        RCF_DTOR_END;
    }

    AsioNetworkSessionPtr AsioNetworkSession::sharedFromThis()
    {
        return boost::static_pointer_cast<AsioNetworkSession>(shared_from_this());
    }

    ByteBuffer AsioNetworkSession::getReadByteBuffer()
    {
        if (!mAppReadBufferPtr)
        {
            return ByteBuffer();            
        }
        return ByteBuffer(mAppReadBufferPtr);
    }

    void AsioNetworkSession::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        if (byteBuffer.getLength() == 0 && bytesRequested > 0)
        {
            if (!mNetworkReadBufferPtr || mNetworkReadBufferPtr.unique())
            {
                mNetworkReadBufferPtr = getObjectPool().getReallocBufferPtr();
            }
            mNetworkReadBufferPtr->resize(bytesRequested);
            mNetworkReadByteBuffer = ByteBuffer(mNetworkReadBufferPtr);
        }
        else
        {
            mNetworkReadByteBuffer = ByteBuffer(byteBuffer, 0, bytesRequested);
        }

        RCF_ASSERT_LTEQ(bytesRequested, mNetworkReadByteBuffer.getLength());

        char *buffer = mNetworkReadByteBuffer.getPtr();
        std::size_t bufferLen = mNetworkReadByteBuffer.getLength();

        if (mSocketOpsMutexPtr)
        {
            Lock lock(*mSocketOpsMutexPtr);
            implRead(buffer, bufferLen);
        }
        else
        {
            implRead(buffer, bufferLen);
        }
    }

    void AsioNetworkSession::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        RCF_ASSERT(!byteBuffers.empty());

        if ( mSocketOpsMutexPtr )
        {
            Lock lock(*mSocketOpsMutexPtr);
            implWrite(byteBuffers);
        }
        else
        {
            implWrite(byteBuffers);
        }
    }

    // TODO: merge onReadCompletion/onWriteCompletion into one function

    void AsioNetworkSession::onNetworkReadCompleted(
        AsioErrorCode error, size_t bytesTransferred)
    {
        RCF_LOG_4()(this)(bytesTransferred) << "AsioNetworkSession - read from socket completed.";

        ThreadTouchGuard threadTouchGuard;

        mLastError = error;

        mBytesReceivedCounter += bytesTransferred;

#ifdef BOOST_WINDOWS

        if (error.value() == ERROR_OPERATION_ABORTED)
        {
            error = AsioErrorCode();
        }

#endif

        if (!error && !mTransport.mStopFlag)
        {
            if (bytesTransferred == 0 && mIssueZeroByteRead)
            {
                // TCP framing.
                if (!mAppReadBufferPtr || !mAppReadBufferPtr.unique())
                {
                    mAppReadBufferPtr = getObjectPool().getReallocBufferPtr();
                }
                mAppReadBufferPtr->resize(4);

                mReadBufferRemaining = 4;
                mIssueZeroByteRead = false;
                beginRead();
            }
            else
            {
                CurrentRcfSessionSentry guard(mRcfSessionPtr);

                mNetworkReadByteBuffer = ByteBuffer(
                    mNetworkReadByteBuffer,
                    0,
                    bytesTransferred);

                mTransportFilters.empty() ?
                    onAppReadWriteCompleted(bytesTransferred) : 
                    mTransportFilters.back()->onReadCompleted(mNetworkReadByteBuffer);
            }
        }
    }

    void AsioNetworkSession::onNetworkWriteCompleted(
        AsioErrorCode error, 
        size_t bytesTransferred)
    {
        RCF_LOG_4()(this)(bytesTransferred) << "AsioNetworkSession - write to socket completed.";

        ThreadTouchGuard threadTouchGuard;

        mLastError = error;

        mBytesSentCounter += bytesTransferred;

#ifdef BOOST_WINDOWS

        if (error.value() == ERROR_OPERATION_ABORTED)
        {
            error = AsioErrorCode();
        }

#endif

        if (!error && !mTransport.mStopFlag)
        {
            CurrentRcfSessionSentry guard(mRcfSessionPtr);

            mTransportFilters.empty() ?
                onAppReadWriteCompleted(bytesTransferred) :
                mTransportFilters.back()->onWriteCompleted(bytesTransferred);
        }
    }

    void AsioNetworkSession::setTransportFilters(
        const std::vector<FilterPtr> &filters)
    {
        mTransportFilters.assign(filters.begin(), filters.end());

        std::copy(
            mWireFilters.begin(), 
            mWireFilters.end(), 
            std::back_inserter(mTransportFilters));

        connectFilters(mTransportFilters);

        if (!mTransportFilters.empty())
        {
            mTransportFilters.front()->setPreFilter( *mFilterAdapterPtr );
            mTransportFilters.back()->setPostFilter( *mFilterAdapterPtr );
        }
    }

    void AsioNetworkSession::getTransportFilters(
        std::vector<FilterPtr> &filters)
    {
        filters = mTransportFilters;
        for (std::size_t i=0; i<mWireFilters.size(); ++i)
        {
            filters.pop_back();
        }
    }

    void AsioNetworkSession::setWireFilters(
        const std::vector<FilterPtr> &filters)
    {
        std::vector<FilterPtr> transportFilters;
        getTransportFilters(transportFilters);

        mWireFilters = filters;
        setTransportFilters(transportFilters);
    }

    void AsioNetworkSession::getWireFilters(
        std::vector<FilterPtr> &filters)
    {
        filters = mWireFilters;
    }


    void AsioNetworkSession::beginRead()
    {
        if ( mCloseAfterWrite )
        {
            return;
        }

        RCF_ASSERT(
                mReadBufferRemaining == 0 
            ||  (mAppReadBufferPtr && mAppReadBufferPtr->size() >= mReadBufferRemaining));

        mAppReadByteBuffer = ByteBuffer();
        if (mAppReadBufferPtr)
        {
            char * pch = & (*mAppReadBufferPtr)[mAppReadBufferPtr->size() - mReadBufferRemaining];
            mAppReadByteBuffer = ByteBuffer(pch, mReadBufferRemaining, mAppReadBufferPtr);
        }

        mTransportFilters.empty() ?
            read(mAppReadByteBuffer, mReadBufferRemaining) :
            mTransportFilters.front()->read(mAppReadByteBuffer, mReadBufferRemaining);
    }

    void AsioNetworkSession::beginWrite()
    {
        mSlicedWriteByteBuffers.resize(0);

        sliceByteBuffers(
            mSlicedWriteByteBuffers,
            mWriteByteBuffers,
            lengthByteBuffers(mWriteByteBuffers)-mWriteBufferRemaining);

        mTransportFilters.empty() ?
            write(mSlicedWriteByteBuffers) :
            mTransportFilters.front()->write(mSlicedWriteByteBuffers);
    }

    void AsioNetworkSession::beginAccept()
    {
        mState = Accepting;
        implAccept();
    }

    void AsioNetworkSession::onAcceptCompleted(
        const AsioErrorCode & error)
    {
        RCF_LOG_4()(error.value())
            << "AsioNetworkSession - onAccept().";

        if (mTransport.mStopFlag)
        {
            RCF_LOG_4()(error.value())
                << "AsioNetworkSession - onAccept(). Returning early, stop flag is set.";

            return;
        }

        // create a new NetworkSession, and do an accept on that
        mTransport.createNetworkSession()->beginAccept();

        if (!error)
        {
            // save the remote address in the NetworkSession object
            bool clientAddrAllowed = implOnAccept();
            mState = WritingData;

            // set current RCF session
            mRcfSessionPtr = mTransport.getSessionManager().createSession();
            mRcfSessionPtr->setNetworkSession( *this );
            CurrentRcfSessionSentry guard(mRcfSessionPtr);

            if (clientAddrAllowed)
            {
                // Check the connection limit.
                bool allowConnect = true;
                std::size_t connectionLimit = mTransport.getConnectionLimit();
                if (connectionLimit)
                {
                    Lock lock(mTransport.mSessionsMutex);
                    
                    RCF_ASSERT_LTEQ(
                        mTransport.mSessions.size() , 1+1+connectionLimit);

                    if (mTransport.mSessions.size() == 1+1+connectionLimit)
                    {
                        allowConnect = false;
                    }
                }

                if (allowConnect)
                {
                    // start things rolling by faking a completed write operation
                    onAppReadWriteCompleted(0);
                }
                else
                {
                    sendServerError(RcfError_ConnectionLimitExceeded);
                }
            }
        }
    }

    void onError(
        AsioErrorCode & error1, 
        const AsioErrorCode & error2)
    {
        error1 = error2;
    }

    void AsioNetworkSession::sendServerError(int error)
    {
        mState = Ready;
        mCloseAfterWrite = true;
        std::vector<ByteBuffer> byteBuffers(1);
        encodeServerError(*mTransport.mpServer, byteBuffers.front(), error);
        postWrite(byteBuffers);
    }

    void AsioNetworkSession::doRegularFraming(size_t bytesTransferred)
    {
        RCF_ASSERT_LTEQ(bytesTransferred , mReadBufferRemaining);
        mReadBufferRemaining -= bytesTransferred;
        if (mReadBufferRemaining > 0)
        {
            beginRead();
        }
        else if (mReadBufferRemaining == 0 && mIssueZeroByteRead)
        {
            if (!mAppReadBufferPtr || !mAppReadBufferPtr.unique())
            {
                mAppReadBufferPtr = getObjectPool().getReallocBufferPtr();
            }
            mAppReadBufferPtr->resize(4);

            mReadBufferRemaining = 4;
            mIssueZeroByteRead = false;
            beginRead();
        }
        else
        {
            RCF_ASSERT_EQ(mReadBufferRemaining , 0);
            if (mState == ReadingDataCount)
            {
                ReallocBuffer & readBuffer = *mAppReadBufferPtr;
                RCF_ASSERT_EQ(readBuffer.size() , 4);

                unsigned int packetLength = 0;
                memcpy(&packetLength, &readBuffer[0], 4);
                networkToMachineOrder(&packetLength, 4, 1);

                if (    mTransport.getMaxMessageLength() 
                    &&  packetLength > mTransport.getMaxMessageLength())
                {
                    sendServerError(RcfError_ServerMessageLength);
                }
                else
                {
                    readBuffer.resize(packetLength);
                    mReadBufferRemaining = packetLength;
                    mState = ReadingData;
                    beginRead();
                }
            }
            else if (mState == ReadingData)
            {
                mState = Ready;

                mTransport.getSessionManager().onReadCompleted(
                    getSessionPtr());
            }
        }
    }

    void AsioNetworkSession::doCustomFraming(size_t bytesTransferred)
    {
        RCF_ASSERT_LTEQ(bytesTransferred , mReadBufferRemaining);
        mReadBufferRemaining -= bytesTransferred;
        if (mReadBufferRemaining > 0)
        {
            beginRead();
        }
        else if (mReadBufferRemaining == 0 && mIssueZeroByteRead)
        {
            if (!mAppReadBufferPtr || !mAppReadBufferPtr.unique())
            {
                mAppReadBufferPtr = getObjectPool().getReallocBufferPtr();
            }
            mAppReadBufferPtr->resize(4);

            mReadBufferRemaining = 4;
            mIssueZeroByteRead = false;
            beginRead();
        }
        else
        {
            RCF_ASSERT_EQ(mReadBufferRemaining , 0);
            if (mState == ReadingDataCount)
            {
                ReallocBuffer & readBuffer = *mAppReadBufferPtr;
                RCF_ASSERT_EQ(readBuffer.size() , 4);

                std::size_t messageLength = mTransportFilters[0]->getFrameSize();

                if (    mTransport.getMaxMessageLength() 
                    &&  messageLength > mTransport.getMaxMessageLength())
                {
                    sendServerError(RcfError_ServerMessageLength);
                }
                else
                {
                    RCF_ASSERT( messageLength > 4 );
                    readBuffer.resize(messageLength);
                    mReadBufferRemaining = messageLength - 4;
                    mState = ReadingData;
                    beginRead();
                }
            }
            else if (mState == ReadingData)
            {
                mState = Ready;

                mTransport.getSessionManager().onReadCompleted(
                    getSessionPtr());
            }
        }
    }

    void AsioNetworkSession::onAppReadWriteCompleted(
        size_t bytesTransferred)
    {
        setLastActivityTimestamp();

        switch(mState)
        {
        case ReadingDataCount:
        case ReadingData:

            if (mTransport.mCustomFraming)
            {
                doCustomFraming(bytesTransferred);
            }
            else
            {
                doRegularFraming(bytesTransferred);
            }
            
            break;

        case WritingData:

            RCF_ASSERT_LTEQ(bytesTransferred , mWriteBufferRemaining);

            mWriteBufferRemaining -= bytesTransferred;
            if (mWriteBufferRemaining > 0)
            {
                beginWrite();
            }
            else
            {
                if (mCloseAfterWrite)
                {
                    // For TCP sockets, call shutdown() so client receives 
                    // the message before we close the connection.

                    implCloseAfterWrite();                        
                }
                else
                {
                    mState = Ready;

                    mSlicedWriteByteBuffers.resize(0);
                    mWriteByteBuffers.resize(0);

                    mTransport.getSessionManager().onWriteCompleted(
                        getSessionPtr());
                }
            }
            break;

        default:
            RCF_ASSERT(0);
        }
    }

    // AsioServerTransport

    AsioNetworkSessionPtr AsioServerTransport::createNetworkSession()
    {
        AsioNetworkSessionPtr networkSessionPtr( implCreateNetworkSession() );
        networkSessionPtr->mWeakThisPtr = networkSessionPtr;
        registerSession(networkSessionPtr->mWeakThisPtr);
        return networkSessionPtr;
    }

    // I_ServerTransportEx implementation

    ClientTransportAutoPtr AsioServerTransport::createClientTransport(
        const Endpoint &endpoint)
    {
        return implCreateClientTransport(endpoint);
    }

    SessionPtr AsioServerTransport::createServerSession(
        ClientTransportAutoPtr & clientTransportAutoPtr,
        StubEntryPtr stubEntryPtr,
        bool keepClientConnection)
    {
        // Create a new network session.
        AsioNetworkSessionPtr networkSessionPtr(createNetworkSession());
        
        // Create a RCF session for the network session.
        SessionPtr sessionPtr = getSessionManager().createSession();
        sessionPtr->setIsCallbackSession(true);
        sessionPtr->setNetworkSession(*networkSessionPtr);
        networkSessionPtr->mRcfSessionPtr = sessionPtr;

        // Move socket from client transport to network session.
        networkSessionPtr->implTransferNativeFrom(*clientTransportAutoPtr);

        if (stubEntryPtr)
        {
            sessionPtr->setDefaultStubEntryPtr(stubEntryPtr);
        }

        // Copy over the wire filters from the client transport.
        ConnectedClientTransport& clientTransport = 
            static_cast<ConnectedClientTransport&>(*clientTransportAutoPtr);

        bool doingHttp = false;

        // If wire filters are present, some special gymnastics are needed. For now
        // we assume that the presence of wire filters indicates an HTTP connection.
        if ( clientTransport.mWireFilters.size() > 0 )
        {
            doingHttp = true;

            std::size_t wireFilterCount = clientTransport.mWireFilters.size();
            RCF_ASSERT(wireFilterCount == 1 || wireFilterCount == 3);
            RCF_UNUSED_VARIABLE(wireFilterCount);
            networkSessionPtr->setWireFilters(clientTransport.mWireFilters);
        }

        // Create a new controlling client transport, if applicable.
        clientTransportAutoPtr.reset();
        if ( keepClientConnection && !doingHttp )
        {
            clientTransportAutoPtr.reset( createClientTransport(sessionPtr).release() );
        }

        // Start reading on the server session.
        networkSessionPtr->mState = AsioNetworkSession::WritingData;
        networkSessionPtr->onAppReadWriteCompleted(0);
        return sessionPtr;
    }

    ClientTransportAutoPtr AsioServerTransport::createClientTransport(
        SessionPtr sessionPtr)
    {
        // If wire filters are present, some special gymnastics are needed. For now
        // we assume that the presence of wire filters indicates an HTTP connection.

        AsioNetworkSession & networkSession =
            dynamic_cast<AsioNetworkSession &>(sessionPtr->getNetworkSession());

        bool doingHttp = false;
        if ( networkSession.mWireFilters.size() > 0 )
        {
            doingHttp = true;
            
            // TODO: better check for HTTP/HTTPS.
            std::size_t wireFilterCount = networkSession.mWireFilters.size();
            RCF_ASSERT(wireFilterCount == 2 || wireFilterCount == 3);
            RCF_UNUSED_VARIABLE(wireFilterCount);

            FilterPtr httpFrameFilterPtr = networkSession.mWireFilters[1];
            HttpFrameFilter& httpFrame = static_cast<HttpFrameFilter&>(*httpFrameFilterPtr);
            httpFrame.mChunkedResponseMode = true;
            httpFrame.mChunkedResponseCounter = 0;
        }

        // Make sure the network session stays alive for the time being.
        AsioNetworkSessionPtr networkSessionPtr = networkSession.sharedFromThis();

        ClientTransportAutoPtr clientTransportPtr = networkSession.implCreateClientTransport();
        ConnectedClientTransport & connClientTransport = static_cast<ConnectedClientTransport &>(*clientTransportPtr);
        connClientTransport.setRcfSession(networkSession.mRcfSessionPtr);

        networkSession.mSocketOpsMutexPtr.reset( new Mutex() );
        connClientTransport.setSocketOpsMutex(networkSession.mSocketOpsMutexPtr);

        if ( doingHttp )
        {
            // Drop the HTTP session filter at the front of the wire filter sequence.
            std::vector<FilterPtr> wireFilters;
            networkSession.getWireFilters(wireFilters);
            wireFilters.erase(wireFilters.begin());
            networkSession.setWireFilters(wireFilters);
        }

        return clientTransportPtr;
    }

    // I_Service implementation

    void AsioServerTransport::open()
    {
        mStopFlag = false;
        implOpen();
    }


    void AsioNetworkSession::close()
    {
        implClose();
    }

    AsioErrorCode AsioNetworkSession::getLastError()
    {
        return mLastError;
    }

    void AsioNetworkSession::setCloseAfterWrite()
    {
        mCloseAfterWrite = true;
    }

    void AsioServerTransport::close()
    {
        mAcceptorPtr.reset();
        mStopFlag = true;
        cancelOutstandingIo();

        mpIoService = NULL;
        mpServer = NULL;
    }

    void AsioServerTransport::stop()
    {
        mpIoService->stop();
    }

    void AsioServerTransport::onServiceAdded(RcfServer &server)
    {
        setServer(server);
        mTaskEntries.clear();
        mTaskEntries.push_back(TaskEntry(Mt_Asio));
    }

    void AsioServerTransport::onServiceRemoved(RcfServer &)
    {}

    void AsioServerTransport::onServerStart(RcfServer & server)
    {
        open();

        mStopFlag = false;
        mpServer  = &server;
        mpIoService = mTaskEntries[0].getThreadPool().getIoService();        
    }

    void AsioServerTransport::startAcceptingThread(Exception & eRet)
    {
        try
        {
            std::size_t initialNumberOfConnections = getInitialNumberOfConnections();
            for (std::size_t i=0; i<initialNumberOfConnections; ++i)
            {
                createNetworkSession()->beginAccept();
            }
        }
        catch(const Exception & e)
        {
            eRet = e;
        }
        catch(const std::exception & e)
        {
            eRet = Exception(e.what());
        }
        catch(...)
        {
            eRet = Exception("Unknown exception type caught in AsioServerTransport::startAcceptingThread().");
        }
    }

    void AsioServerTransport::startAccepting()
    {
        bool runningOnWindowsVistaOrLater = false;

#ifdef BOOST_WINDOWS
        OSVERSIONINFO osvi = {0};
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // warning C4996: 'GetVersionExA': was declared deprecated
#endif

        GetVersionEx(&osvi);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

        if (osvi.dwMajorVersion >= 6)
        {
            runningOnWindowsVistaOrLater = true;
        }
#endif

        Exception e;
        if (runningOnWindowsVistaOrLater)
        {
            // http://stackoverflow.com/questions/12047960/getqueuedcompletionstatus-cant-dequeue-io-from-iocp-if-the-thread-which-origina

            // Due to a bug in Windows 8, we need to make the async accept calls
            // on a thread that won't subsequently block on I/O. If we do it on
            // this thread, and the user then calls e.g. std::cin.get() after
            // starting the server, none of the accepts will ever complete.

            // To simplify testing, we apply this workaround from Vista onwards. 
            // We can't apply it on earlier Windows (XP, 2003), because async 
            // calls are canceled when the issuing thread terminates.

            Thread t( boost::bind(&AsioServerTransport::startAcceptingThread, this, boost::ref(e)) );
            t.join();
        }
        else
        {
            startAcceptingThread(e);
        }

        if (e.bad())
        {
            e.throwSelf();
        }
    }

    void AsioServerTransport::onServerStop(RcfServer &)
    {
        close();
    }

    void AsioServerTransport::setServer(RcfServer &server)
    {
        mpServer = &server;
    }

    RcfServer & AsioServerTransport::getServer()
    {
        return *mpServer;
    }

    RcfServer & AsioServerTransport::getSessionManager()
    {
        return *mpServer;
    }

    AsioServerTransport::AsioServerTransport() :
        mpIoService(),
        mAcceptorPtr(),
        mWireProtocol(Wp_None),
        mStopFlag(),
        mpServer()
    {
    }

    AsioServerTransport::~AsioServerTransport()
    {
    }

    void AsioServerTransport::registerSession(AsioNetworkSessionWeakPtr session)
    {
        Lock lock(mSessionsMutex);
        mSessions.insert(session);
    }

    void AsioServerTransport::unregisterSession(AsioNetworkSessionWeakPtr session)
    {
        Lock lock(mSessionsMutex);
        mSessions.erase(session);
    }

    void AsioServerTransport::cancelOutstandingIo()
    {
        Lock lock(mSessionsMutex);

        std::set<NetworkSessionWeakPtr>::iterator iter;
        for (iter = mSessions.begin(); iter != mSessions.end(); ++iter)
        {
            NetworkSessionPtr networkSessionPtr = iter->lock();
            if (networkSessionPtr)
            {
                AsioNetworkSessionPtr asioNetworkSessionPtr = 
                    boost::static_pointer_cast<AsioNetworkSession>(networkSessionPtr);

                asioNetworkSessionPtr->close();
            }
        }
    }

    AsioAcceptor & AsioServerTransport::getAcceptor()
    {
        return *mAcceptorPtr;
    }

    AsioIoService & AsioServerTransport::getIoService()
    {
        return *mpIoService;
    }

} // namespace RCF
