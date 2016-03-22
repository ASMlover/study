
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

#include <RCF/Marshal.hpp>

#include <algorithm>

#include <boost/function.hpp>

#include <RCF/AmiThreadPool.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/Future.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/OverlappedAmi.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    bool serializeOverride(SerializationProtocolOut &out, ByteBuffer & u)
    {
        int runtimeVersion = out.getRuntimeVersion();

        if (runtimeVersion <= 3)
        {
            // Legacy behavior - no metadata for ByteBuffer.
            int len = static_cast<int>(u.getLength());
            serialize(out, len);
            out.insert(u);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool serializeOverride(SerializationProtocolOut &out, ByteBuffer * pu)
    {
        RCF_ASSERT(pu);
        return serializeOverride(out, *pu);
    }

    bool deserializeOverride(SerializationProtocolIn &in, ByteBuffer & u)
    {
        int runtimeVersion = in.getRuntimeVersion();

        if (runtimeVersion <= 3)
        {
            // Legacy behavior - no metadata for ByteBuffer.
            int len = 0;
            deserialize(in, len);
            in.extractSlice(u, len);
            return true;
        }
        else
        {
            return false;
        }
    }

    Mutex * gpCandidatesMutex = NULL;
    Candidates * gpCandidates = NULL;

    void initAmi()
    {
        gpCandidatesMutex = new Mutex; 
        gpCandidates = new Candidates;
    }

    void deinitAmi()
    {
        delete gpCandidatesMutex; 
        gpCandidatesMutex = NULL; 
        delete gpCandidates; 
        gpCandidates = NULL;
    }


    Mutex & gCandidatesMutex()
    {
        return *gpCandidatesMutex; 
    }

    Candidates & gCandidates()
    {
        return *gpCandidates;
    }

    void ClientStub::enrol(I_Future *pFuture)
    {
        mFutures.push_back(pFuture);
        pFuture->setClientStub(this);
    }

    void ClientStub::init( 
        const std::string & interfaceName,
        int fnId, 
        RCF::RemoteCallSemantics rcs)
    {
        mRequest.init(
            getTargetToken(),
            getTargetName(),
            interfaceName,
            fnId,
            getSerializationProtocol(),
            mMarshalingProtocol,
            (rcs == RCF::Oneway),
            false,
            getRuntimeVersion(),
            false,
            mPingBackIntervalMs,
            mArchiveVersion,
            mUseNativeWstringSerialization,
            mEnableSfPointerTracking);

        ::RCF::CurrentClientStubSentry sentry(*this);

        mOut.reset(
            getSerializationProtocol(),
            32,
            mRequest.encodeRequestHeader(),
            mRuntimeVersion,
            mArchiveVersion,
            mEnableSfPointerTracking);

        bool asyncParameters = false;
        mpParameters->write(mOut);
        mFutures.clear();
        asyncParameters = mpParameters->enrolFutures(this);

        if (asyncParameters)
        {
            setAsync(true);
        }
    }

    void ClientStub::connect()
    {
        CurrentClientStubSentry sentry(*this);
        instantiateTransport();

        // Do we need to reconnect?

        bool shouldReconnect = false;

        if (!mConnected)
        {
            // Not connected.
            shouldReconnect = true;
        }
        else if (       mConnected 
                    &&  mAutoReconnect 
                    &&  mRcs == Twoway 
                    &&  !mTransport->isConnected())
        {
            // Auto reconnect.
            shouldReconnect = true;
        }

        if (shouldReconnect)
        {
            std::string endpoint;
            if (mEndpoint.get())
            {
                endpoint = mEndpoint->asString();
            }

            RCF_LOG_2()(this)(endpoint)(mConnectTimeoutMs) 
                << "RcfClient - connect to server.";

            if ( getTransportType() == Tt_Http || getTransportType() == Tt_Https )
            {
                if ( getHttpProxy().size() > 0 )
                {
                    RCF_LOG_2()(this) << "RcfClient - HTTP proxy: " << getHttpProxy() << ":" << getHttpProxyPort();
                }
                else
                {
                    RCF_LOG_2()(this) << "RcfClient - HTTP proxy: <None configured>";
                }
            }

            unsigned int connectTimeoutMs = mConnectTimeoutMs;
            if (connectTimeoutMs == 0)
            {
                connectTimeoutMs = MaxTimeoutMs;
            }
            mTransport->disconnect(connectTimeoutMs);

            mAsyncOpType = Connect;

            mTransport->connect(*this, connectTimeoutMs);
        }
        else
        {
            onConnectCompleted(true);
        }
    }

    void ClientStub::connectAsync(boost::function0<void> onCompletion)
    {
        setAsync(true);
        instantiateTransport();
        mTransport->setAsync(mAsync);
        setAsyncCallback(onCompletion);
        connect();

        // In case the connect completes on this thread (as it would for a named pipe).
        getTlsAmiNotification().run();
    }

    void ClientStub::wait(
        boost::function0<void> onCompletion, 
        boost::uint32_t timeoutMs)
    {
        setAsync(true);
        instantiateTransport();
        mTransport->setAsync(mAsync);
        setAsyncCallback(onCompletion);

        mAsyncOpType = Wait;
        mTransport->setTimer(timeoutMs, this);
    }

    void ClientStub::doBatching()
    {
        RCF_ASSERT_EQ(mRcs , Oneway);
        RCF_ASSERT(!mAsync);
        RCF_ASSERT(mBatchBufferPtr);

        // Accumulate in the batch buffer.
        std::size_t appendLen = lengthByteBuffers(mEncodedByteBuffers);
        std::size_t currentSize = mBatchBufferPtr->size();

        bool usingTempBuffer = false;

        // If this message will cause us to exceed the limit, then flush first.
        if (    mBatchMaxMessageLength 
            &&  currentSize + appendLen > mBatchMaxMessageLength)
        {
            mBatchBufferTemp.resize(appendLen);
            copyByteBuffers(mEncodedByteBuffers, & mBatchBufferTemp[0] );
            usingTempBuffer = true;

            unsigned int timeoutMs = generateTimeoutMs(mEndTimeMs);
            flushBatch(timeoutMs);
            currentSize = mBatchBufferPtr->size();
        }

        mBatchBufferPtr->resize( currentSize + appendLen);

        if (usingTempBuffer)
        {
            memcpy(
                & (*mBatchBufferPtr)[currentSize], 
                &mBatchBufferTemp[0], 
                mBatchBufferTemp.size());
        }
        else
        {
            copyByteBuffers(
                mEncodedByteBuffers, 
                & (*mBatchBufferPtr)[currentSize] );
        }

        ++mBatchMessageCount;
    }

    void ClientStub::onRequestTransportFiltersCompleted()
    {
        if (mAsync && hasAsyncException())
        {
            scheduleAmiNotification();
            getTlsAmiNotification().run();
            return;
        }

        try
        {
            mSetTransportProtocol = false;
            mConnected = true;

            // Progress notification when connection is established.
            if (
                mClientProgressPtr.get() &&
                (mClientProgressPtr->mTriggerMask & ClientProgress::Event))
            {
                ClientProgress::Action action = ClientProgress::Continue;

                mClientProgressPtr->mProgressCallback(
                    0,
                    0,
                    ClientProgress::Event,
                    ClientProgress::Connect,
                    action);

                RCF_VERIFY(
                    action != ClientProgress::Cancel,
                    Exception(_RcfError_ClientCancel()));
            }

            // If we're only connecting, mEncodedByteBuffers will be empty.
            if (!mEncodedByteBuffers.empty())
            {
                beginSend();
            }
            else
            {
                if (mAsync)
                {
                    scheduleAmiNotification();
                }
            }   
        }
        catch(const RCF::Exception & e)
        {
            if (mAsync)
            {
                std::auto_ptr<Exception> ePtr(e.clone());
                setAsyncException(ePtr);
                scheduleAmiNotification();
                getTlsAmiNotification().run();
            }
            else
            {
                throw;
            }
        }
    }

    void ClientStub::beginSend()
    {
        RCF_ASSERT( !mEncodedByteBuffers.empty() );

        unsigned int timeoutMs = generateTimeoutMs(mEndTimeMs);

        if (mAsync)
        {
            mAsyncOpType = Write;
        }

        // Add framing (4 byte length prefix).
        int messageSize = 
            static_cast<int>(RCF::lengthByteBuffers(mEncodedByteBuffers));

        ByteBuffer &byteBuffer = mEncodedByteBuffers.front();

        RCF_ASSERT_GTEQ(byteBuffer.getLeftMargin() , 4);
        byteBuffer.expandIntoLeftMargin(4);
        memcpy(byteBuffer.getPtr(), &messageSize, 4);
        RCF::machineToNetworkOrder(byteBuffer.getPtr(), 4, 1);

        if (mBatchMode)
        {
            doBatching();
        }
        else
        {
            int err = getTransport().send(*this, mEncodedByteBuffers, timeoutMs);
            RCF_UNUSED_VARIABLE(err);
        }
    }

    void ClientStub::onConnectCompleted(bool alreadyConnected)
    {
        CurrentClientStubSentry sentry(*this);

        if (alreadyConnected && !mSetTransportProtocol)
        {
            if (mEncodedByteBuffers.size() > 0)
            {
                beginSend();
            }
        }
        else
        {
            if (mAsync)
            {
                mAsyncOpType = None;
            }

            std::vector<FilterPtr> filters;
            mTransport->getTransportFilters(filters);
            if (
                    mTransportProtocol == Tp_Clear 
                &&  !mEnableCompression 
                &&  filters.size() > 0)
            {
                // Custom filter sequence. Setup transport filters again for a 
                // new connection.
                std::vector<FilterPtr> filterVec;
                mTransport->getTransportFilters(filterVec);

                for ( std::size_t i = 0; i<filterVec.size(); ++i )
                {
                    filterVec[i]->resetState();
                }

                mTransport->setTransportFilters(std::vector<FilterPtr>());
                if ( !filterVec.empty() )
                {
                    requestTransportFilters(filterVec);
                }
                onRequestTransportFiltersCompleted();
            }
            else
            {
                if (!mTransport->supportsTransportFilters())
                {
                    onRequestTransportFiltersCompleted();
                }
                else
                {
                    mTransport->setTransportFilters(std::vector<FilterPtr>());
                    if (mTransportProtocol == Tp_Clear && !mEnableCompression)
                    {
                        onRequestTransportFiltersCompleted();
                    }
                    else
                    {
                        std::vector<FilterPtr> filterVec;
                        createFilterSequence(filterVec);

                        if (mAsync)
                        {
                            requestTransportFiltersAsync(
                                filterVec, 
                                boost::bind(&ClientStub::onRequestTransportFiltersCompleted, this));
                        }
                        else
                        {
                            requestTransportFilters(filterVec);
                            onRequestTransportFiltersCompleted();
                        }
                    }
                }
            }
        }
    }

    void ClientStub::beginCall()
    {
        CurrentClientStubSentry sentry(*this);

        unsigned int totalTimeoutMs = getRemoteCallTimeoutMs();
        unsigned int startTimeMs = getCurrentTimeMs();
        if (totalTimeoutMs)
        {
            mEndTimeMs = startTimeMs + totalTimeoutMs;
        }
        else
        {
            // Disable timeout.
            static const unsigned int maxTimeoutMs = (((unsigned int)-1)/10)*9;
            mEndTimeMs = startTimeMs + maxTimeoutMs;
        }

        ThreadLocalCached< std::vector<ByteBuffer> > tlcByteBuffers;
        std::vector<ByteBuffer> &byteBuffers = tlcByteBuffers.get();

        mOut.extractByteBuffers(byteBuffers);
        int protocol = mOut.getSerializationProtocol();
        RCF_UNUSED_VARIABLE(protocol);

        mEncodedByteBuffers.resize(0);

        mRequest.encodeRequest(
            byteBuffers,
            mEncodedByteBuffers,
            getMessageFilters());

        instantiateTransport();

        mTransport->setAsync(mAsync);

        WithProgressCallback *pWithCallbackProgress =
            dynamic_cast<WithProgressCallback *>(&getTransport());

        if (pWithCallbackProgress)
        {
            pWithCallbackProgress->setClientProgressPtr(
                getClientProgressPtr());
        }

        // TODO: make sure timeouts behave as expected, esp. when connect() is 
        // doing round trip filter setup calls
        connect();

    }

    void ClientStub::onSendCompleted()
    {
        if (mRcs == RCF::Oneway)
        {
            mEncodedByteBuffers.resize(0);

            // TODO: Refactor. This code is identical to what happens when a 
            // receive completes, in a twoway call.

            if (mAsync)
            {
                mAsyncOpType = None;
                scheduleAmiNotification();
            }
        }
        else
        {
            mEncodedByteBuffers.resize(0);
            beginReceive();
        }
    }

    void ClientStub::beginReceive()
    {
        if (mPingBackIntervalMs && mRuntimeVersion >= 5)
        {
            mPingBackCheckIntervalMs = 3 * mPingBackIntervalMs;

            mNextPingBackCheckMs = 
                RCF::getCurrentTimeMs() + mPingBackCheckIntervalMs;

            // So we avoid the special value 0.
            mNextPingBackCheckMs |= 1;
        }

        if (mAsync)
        {
            mAsyncOpType = Read;
        }

        unsigned int timeoutMs = generateTimeoutMs(mEndTimeMs);

        mEncodedByteBuffer.clear();
        int err = getTransport().receive(*this, mEncodedByteBuffer, timeoutMs);
        RCF_UNUSED_VARIABLE(err);
    }

    void ClientStub::onException(const Exception & e)
    {
        if (mAsync)
        {
            onError(e);
        }
        else
        {
            e.throwSelf();
        }
    }

    void ClientStub::onReceiveCompleted()
    {
        if (mAsync)
        {
            mAsyncOpType = None;
        }

        ByteBuffer unfilteredByteBuffer;

        MethodInvocationResponse response;

        mRequest.decodeResponse(
            mEncodedByteBuffer,
            unfilteredByteBuffer,
            response,
            getMessageFilters());

        mEncodedByteBuffer.clear();

        mIn.reset(
            unfilteredByteBuffer,
            mOut.getSerializationProtocol(),
            mRuntimeVersion,
            mArchiveVersion,
            response.getEnableSfPointerTracking());

        RCF_LOG_3()(this)(response) << "RcfClient - received response.";

        if (response.isException())
        {
            std::auto_ptr<RemoteException> remoteExceptionAutoPtr(
                response.getExceptionPtr());

            if (!remoteExceptionAutoPtr.get())
            {
                int runtimeVersion = mRequest.mRuntimeVersion;
                if (runtimeVersion < 8)
                {
                    deserialize(mIn, remoteExceptionAutoPtr);
                }
                else
                {
                    RemoteException * pRe = NULL;
                    deserialize(mIn, pRe);
                    remoteExceptionAutoPtr.reset(pRe);
                }
            }

            onException(*remoteExceptionAutoPtr);
        }
        else if (response.isError())
        {
            int err = response.getError();
            if (err == RcfError_VersionMismatch)
            {
                int serverRuntimeVersion = response.getArg0();
                int serverArchiveVersion = response.getArg1();

                int clientRuntimeVersion = getRuntimeVersion();
                int clientArchiveVersion = getArchiveVersion();

                // We get this response from the server, if either the
                // client runtime version or client archive version, is
                // greater than what the server supports.

                if (getAutoVersioning() && getTries() == 0)
                {
                    int adjustedRuntimeVersion = RCF_MIN(serverRuntimeVersion, clientRuntimeVersion);
                    setRuntimeVersion(adjustedRuntimeVersion);

                    
                    if (serverArchiveVersion)
                    {
                        int adjustedArchiveVersion = RCF_MIN(serverArchiveVersion, clientArchiveVersion);
                        setArchiveVersion(adjustedArchiveVersion);
                    }
                    setTries(1);

                    init(mRequest.getSubInterface(), mRequest.getFnId(), mRcs);
                    beginCall();
                }
                else
                {
                    onException( VersioningException(
                        serverRuntimeVersion, 
                        serverArchiveVersion));
                }
            }
            else if (err == RcfError_PingBack)
            {
                // A ping back message carries a parameter specifying
                // the ping back interval in ms. The client can use that
                // to make informed decisions about whether the connection
                // has died or not.

                mPingBackIntervalMs = response.getArg0();

                // Record a timestamp and go back to receiving.

                ++mPingBackCount;
                mPingBackTimeStamp = RCF::getCurrentTimeMs();

                applyRecursionLimiter(
                    mRecursionState,
                    &ClientStub::beginReceive,
                    *this);
            }
            else
            {
                onException(RemoteException( Error(response.getError()) ));
            }
        }
        else
        {
            RCF::CurrentClientStubSentry sentry(*this);
            mpParameters->read(mIn);
            mIn.clearByteBuffer();

#if RCF_FEATURE_FILETRANSFER==1

            // Check for any file downloads.
            {
                if (!mDownloadStreams.empty())
                {
                    std::vector<FileDownload> downloadStreams;
                    downloadStreams.swap(mDownloadStreams);
                    for (std::size_t i=0; i<downloadStreams.size(); ++i)
                    {
                        downloadStreams[i].download(*this);
                    }
                }
            }

#endif

            if (mAsync)
            {
                scheduleAmiNotification();
            }
        }
    }

    void ClientStub::scheduleAmiNotification()
    {
        if (!mCurrentCallDesc.empty())
        {
            RCF_LOG_2() << "RcfClient - end remote call. " << mCurrentCallDesc;
        }

        if (mSignalledLockPtr)
        {
            RCF_ASSERT( !mSignalledLockPtr->locked() );
            mSignalledLockPtr->lock();
        }
        else
        {
            mSignalledLockPtr.reset( new Lock(*mSignalledMutexPtr) );
        }

        mCallInProgress = false;
        mSignalled = true;
        mSignalledConditionPtr->notify_all(*mSignalledLockPtr);

        boost::function0<void> cb;
        if (mAsyncCallback)
        {
            cb = mAsyncCallback;
            mAsyncCallback = boost::function0<void>();                
        }

        getTlsAmiNotification().set(cb, mSignalledLockPtr, mSignalledMutexPtr);
    }

    bool ClientStub::ready()
    {
        Lock lock(*mSignalledMutexPtr);
        return mSignalled;
    }

    void ClientStub::waitForReady(boost::uint32_t timeoutMs)
    {
        Lock lock(*mSignalledMutexPtr);
        if (!mSignalled)
        {
            if (timeoutMs)
            {
                mSignalledConditionPtr->timed_wait(lock, timeoutMs);
            }
            else
            {
                mSignalledConditionPtr->wait(lock);
            }            
        }
    }

    void ClientStub::cancel()
    {
        if (mTransport.get())
        {
            mTransport->cancel();
        }
        getTlsAmiNotification().run();
    }

    void ClientStub::setSubRcfClientPtr(RcfClientPtr clientStubPtr)
    {
        Lock lock(mSubRcfClientMutex);
        mSubRcfClientPtr = clientStubPtr;
    }

    RcfClientPtr ClientStub::getSubRcfClientPtr()
    {
        Lock lock(mSubRcfClientMutex);
        return mSubRcfClientPtr;
    }

    void ClientStub::call( 
        RCF::RemoteCallSemantics rcs)
    {

        if (    rcs == RCF::Oneway 
            &&  (getTransportType() == RCF::Tt_Http || getTransportType() == RCF::Tt_Https) )
        {
            // Oneway is not possible over HTTP/HTTPS.
            throw RCF::Exception(_RcfError_OnewayHttp());
        }

        mRetry = false;
        mRcs = rcs;
        mPingBackTimeStamp = 0;
        mPingBackCount = 0;

        // Set the progress timer timeouts.
        mTimerIntervalMs = 0;
        mNextTimerCallbackMs = 0;

        if (    mClientProgressPtr.get()
            &&  mClientProgressPtr->mTriggerMask & ClientProgress::Timer)
        {            
            mTimerIntervalMs = mClientProgressPtr->mTimerIntervalMs;

            mNextTimerCallbackMs = 
                RCF::getCurrentTimeMs() + mTimerIntervalMs;

            // So we avoid the special value 0.
            mNextTimerCallbackMs |= 1;
        }

        // We don't set ping back timeouts until we are about to receive.
        mPingBackCheckIntervalMs = 0;
        mNextPingBackCheckMs = 0;

        mSignalled = false;
        
        beginCall();
    }

    void ClientStub::setAsync(bool async)
    {
        mAsync = async;

        if (mAsync && !mSignalledMutexPtr)
        {
            mSignalledMutexPtr.reset( new Mutex() );
            mSignalledConditionPtr.reset( new Condition() );
        }
    }

    bool ClientStub::getAsync()
    {
        return mAsync;
    }

    void ClientStub::setAsyncCallback(boost::function0<void> callback)
    {
        mAsyncCallback = callback;
    }

    std::auto_ptr<Exception> ClientStub::getAsyncException()
    {
        Lock lock(*mSignalledMutexPtr);
        return mAsyncException;
    }

    void ClientStub::setAsyncException(std::auto_ptr<Exception> asyncException)
    {
        Lock lock(*mSignalledMutexPtr);
        mAsyncException = asyncException;
    }

    bool ClientStub::hasAsyncException()
    {
        Lock lock(*mSignalledMutexPtr);
        return mAsyncException.get() != NULL;
    }

    typedef boost::shared_ptr< ClientTransportAutoPtr > ClientTransportAutoPtrPtr;

    void vc6_helper(
        boost::function2<void, RcfSessionPtr, ClientTransportAutoPtr> func,
        RcfSessionPtr sessionPtr,
        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr)
    {
        func(sessionPtr, *clientTransportAutoPtrPtr);
    }

    void convertRcfSessionToRcfClient(
        OnCallbackConnectionCreated func,
        RemoteCallSemantics rcs)
    {
        RcfSessionPtr sessionPtr = getCurrentRcfSessionPtr()->shared_from_this();
        RcfSession & rcfSession = *sessionPtr;

        ServerTransportEx & serverTransport =
            dynamic_cast<ServerTransportEx &>(
                rcfSession.getNetworkSession().getServerTransport());

        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
            new ClientTransportAutoPtr(
                serverTransport.createClientTransport(rcfSession.shared_from_this())));

        rcfSession.addOnWriteCompletedCallback(
            boost::bind(
                vc6_helper,
                func,
                sessionPtr,
                clientTransportAutoPtrPtr) );

        bool closeSession = (rcs == RCF::Twoway);

        rcfSession.setCloseSessionAfterWrite(closeSession);
    }

    RcfSessionPtr convertRcfClientToRcfSession(
        ClientStub & clientStub, 
        ServerTransport & serverTransport,
        bool keepClientConnection)
    {
        ServerTransportEx &serverTransportEx =
            dynamic_cast<RCF::ServerTransportEx &>(serverTransport);

        ClientTransportAutoPtr clientTransportAutoPtr(
            clientStub.releaseTransport());

        SessionPtr sessionPtr = serverTransportEx.createServerSession(
            clientTransportAutoPtr,
            StubEntryPtr(),
            keepClientConnection);

        clientStub.setTransport(clientTransportAutoPtr);

        return sessionPtr;
    }

    RcfSessionPtr convertRcfClientToRcfSession(
        ClientStub & clientStub, 
        RcfServer & server,
        bool keepClientConnection)
    {
        return convertRcfClientToRcfSession(
            clientStub, 
            server.getServerTransport(), 
            keepClientConnection);
    }

    void createCallbackConnectionImpl(
        ClientStub & clientStubOrig, 
        ServerTransport & callbackServer)
    {
        if (clientStubOrig.getRuntimeVersion() <= 11)
        {
            createCallbackConnectionImpl_Legacy(clientStubOrig, callbackServer);
        }
        else
        {
            I_RcfClient client("", clientStubOrig);
            ClientStub & stub = client.getClientStub();
            stub.setTransport( clientStubOrig.releaseTransport() );

            OobCreateCallbackConnection msg(clientStubOrig.getRuntimeVersion());
            ByteBuffer controlRequest;
            msg.encodeRequest(controlRequest);
            stub.setOutofBandRequest(controlRequest);

            stub.ping(RCF::Twoway);

            // Get OOB response.
            ByteBuffer controlResponse = stub.getOutOfBandResponse();
            stub.setOutofBandRequest(ByteBuffer());
            stub.setOutofBandResponse(ByteBuffer());
            msg.decodeResponse(controlResponse);

            int ret = msg.mResponseError; 
            RCF_VERIFY(ret == RcfError_Ok, RemoteException( Error(ret) ));

            convertRcfClientToRcfSession(client.getClientStub(), callbackServer);
        }
    }

    void createCallbackConnectionImpl(
        ClientStub & clientStub, 
        RcfServer & callbackServer)
    {
        createCallbackConnectionImpl(clientStub, callbackServer.getServerTransport());
    }

    void setCurrentCallDesc(
        std::string& desc, 
        RCF::MethodInvocationRequest& request, 
        const char * szFunc, 
        const char * szArity)
    {
        desc.clear();
        if (LogManager::instance().isEnabled(LogNameRcf, LogLevel_2))
        {
            const std::string & subInterface = request.getSubInterface();
            const std::string & target = request.getService();

            if (subInterface == target)
            {
                desc = request.getSubInterface();
                desc += "::";
                desc += szFunc;
                desc += "().";
            }
            else
            {
                desc = request.getSubInterface();
                desc += "::";
                desc += szFunc;
                if (request.getService().empty())
                {
                    desc += "().";
                }
                else
                {
                    desc += "() on ";
                    desc += request.getService();
                    desc += ".";
                }
            }

            char szFnid[10] = {0};

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4995) // 'sprintf': name was marked as #pragma deprecated
#pragma warning(disable: 4996) // 'sprintf'/'localtime': This function or variable may be unsafe.
#endif

            sprintf(szFnid, "%d", request.getFnId());

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

            desc += " Fnid: ";
            desc += szFnid;
            desc += ". Type: ";
            desc += szArity;
        }
    }

} // namespace RCF
