
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

#include <RCF/RcfServer.hpp>

#include <algorithm>

#include <boost/bind.hpp>

#include <RCF/CallbackConnectionService.hpp>
#include <RCF/Config.hpp>
#include <RCF/Filter.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/Endpoint.hpp>
#include <RCF/FilterService.hpp>
#include <RCF/IpServerTransport.hpp>
#include <RCF/Marshal.hpp>
#include <RCF/MethodInvocation.hpp>

#include <RCF/RcfClient.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ServerTask.hpp>
#include <RCF/Service.hpp>
#include <RCF/StubEntry.hpp>
#include <RCF/TcpClientTransport.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Token.hpp>
#include <RCF/Tools.hpp>
#include <RCF/UdpServerTransport.hpp>

#include <RCF/HttpSessionFilter.hpp>

#include <RCF/Version.hpp>

#if RCF_FEATURE_SF==1
#include <SF/memory.hpp>
#endif

#if RCF_FEATURE_BOOST_SERIALIZATION==1
#include <RCF/BsAutoPtr.hpp>
#endif

#if RCF_FEATURE_FILETRANSFER==1
#include <RCF/FileTransferService.hpp>
#else
namespace RCF { class FileTransferService {}; }
#endif

#if RCF_FEATURE_ZLIB==1
#include <RCF/ZlibCompressionFilter.hpp>
#endif

#if RCF_FEATURE_OPENSSL==1
#include <RCF/OpenSslEncryptionFilter.hpp>
#endif

#if RCF_FEATURE_JSON==1
#include <RCF/JsonRpc.hpp>
#endif

#if RCF_FEATURE_SSPI==1
#include <RCF/Schannel.hpp>
#include <RCF/SspiFilter.hpp>
#endif

#if RCF_FEATURE_NAMEDPIPE==1
#include <RCF/Win32NamedPipeServerTransport.hpp>
#endif

#ifdef BOOST_WINDOWS
#include <RCF/Win32Username.hpp>
#endif

#if RCF_FEATURE_PUBSUB==1
#include <RCF/PublishingService.hpp>
#include <RCF/SubscriptionService.hpp>
#endif

#if RCF_FEATURE_SERVER==1
#include <RCF/CallbackConnectionService.hpp>
#include <RCF/PingBackService.hpp>
#include <RCF/SessionTimeoutService.hpp>
#endif

#if RCF_FEATURE_LEGACY==1
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/SessionObjectFactoryService.hpp>
#endif

namespace RCF {

    // RcfServer

    RcfServer::RcfServer() :
        mStubMapMutex(WriterPriority),
        mStarted(),
        mThreadPoolPtr( new ThreadPool(1) ),
        mRuntimeVersion(RCF::getDefaultRuntimeVersion()),
        mArchiveVersion(RCF::getDefaultArchiveVersion()),
        mPropertiesMutex(WriterPriority)
    {
        mThreadPoolPtr->setThreadName("RCF Server");
        init();
    }

    RcfServer::RcfServer(const Endpoint &endpoint) :
        mStubMapMutex(WriterPriority),
        mStarted(),
        mThreadPoolPtr( new ThreadPool(1) ),
        mRuntimeVersion(RCF::getDefaultRuntimeVersion()),
        mArchiveVersion(RCF::getDefaultArchiveVersion()),
        mPropertiesMutex(WriterPriority)
    {
        mThreadPoolPtr->setThreadName("RCF Server");
        addEndpoint(endpoint);
        init();
    }

    RcfServer::RcfServer(ServicePtr servicePtr) :
        mStubMapMutex(WriterPriority),
        mStarted(),
        mThreadPoolPtr( new ThreadPool(1) ),
        mRuntimeVersion(RCF::getDefaultRuntimeVersion()),
        mArchiveVersion(RCF::getDefaultArchiveVersion()),
        mPropertiesMutex(WriterPriority)
    {
        mThreadPoolPtr->setThreadName("RCF Server");
        addService(servicePtr);
        init();
    }

    RcfServer::RcfServer(ServerTransportPtr serverTransportPtr) :
        mStubMapMutex(WriterPriority),
        mStarted(),
        mThreadPoolPtr( new ThreadPool(1) ),
        mRuntimeVersion(RCF::getDefaultRuntimeVersion()),
        mArchiveVersion(RCF::getDefaultArchiveVersion()),
        mPropertiesMutex(WriterPriority)
    {
        mThreadPoolPtr->setThreadName("RCF Server");
        addService( boost::dynamic_pointer_cast<I_Service>(serverTransportPtr) );
        init();
    }

    RcfServer::~RcfServer()
    {
        RCF_DTOR_BEGIN
            stop();

            mThreadPoolPtr.reset();

            while (mServices.size() > 0)
            {
                mServices.pop_back();
            }

            while (mServerTransports.size() > 0)
            {
                mServerTransports.pop_back();
            }

        RCF_DTOR_END
    }

    void RcfServer::init()
    {
        mSessionTimeoutMs = 0;
        mSessionHarvestingIntervalMs = 30*1000;

        mHttpSessionTimeoutMs = 5*60*1000;

        mServerObjectHarvestingIntervalS = 60;

        mOfsMaxNumberOfObjects = 0;
        mOfsObjectTimeoutS = 0;
        mOfsCleanupIntervalS = 0;
        mOfsCleanupThreshold = 0.0;

        mSslImplementation = RCF::getDefaultSslImplementation();

#if RCF_FEATURE_FILETRANSFER==1
        mFileUploadQuota = 0;
        mFileDownloadQuota = 0;
#endif
        
        mFilterServicePtr.reset( new FilterService() );

#if RCF_FEATURE_SSPI==1
        mFilterServicePtr->addFilterFactory( FilterFactoryPtr( new NtlmFilterFactory() ) );
        mFilterServicePtr->addFilterFactory( FilterFactoryPtr( new KerberosFilterFactory() ) );
        mFilterServicePtr->addFilterFactory( FilterFactoryPtr( new NegotiateFilterFactory() ) );
        mFilterServicePtr->addFilterFactory( FilterFactoryPtr( new SchannelFilterFactory() ) );
#endif

#if RCF_FEATURE_ZLIB==1
        mFilterServicePtr->addFilterFactory( FilterFactoryPtr( new ZlibCompressionFilterFactory() ) );
        mFilterServicePtr->addFilterFactory( FilterFactoryPtr( new ZlibStatelessCompressionFilterFactory() ) );
#endif

#if RCF_FEATURE_OPENSSL==1
        mFilterServicePtr->addFilterFactory( FilterFactoryPtr( new OpenSslEncryptionFilterFactory() ));        
#endif

        addService(mFilterServicePtr);

#if RCF_FEATURE_SERVER==1
        mPingBackServicePtr.reset( new PingBackService() );
        mSessionTimeoutServicePtr.reset( new SessionTimeoutService() );
        mCallbackConnectionServicePtr.reset( new CallbackConnectionService() );
        mServerObjectServicePtr.reset( new ServerObjectService() );
        
        addService(mPingBackServicePtr);
        addService(mSessionTimeoutServicePtr);
        addService(mCallbackConnectionServicePtr);
        addService(mServerObjectServicePtr);
#endif

#if RCF_FEATURE_PUBSUB==1
        mPublishingServicePtr.reset( new PublishingService() );
        mSubscriptionServicePtr.reset( new SubscriptionService() );

        addService(mPublishingServicePtr);
        addService(mSubscriptionServicePtr);
#endif

#if RCF_FEATURE_LEGACY==1
        mSessionObjectFactoryServicePtr.reset( new SessionObjectFactoryService() );
        mObjectFactoryServicePtr.reset( new ObjectFactoryService() );

        addService(mSessionObjectFactoryServicePtr);
        addService(mObjectFactoryServicePtr);
#endif

#if RCF_FEATURE_FILETRANSFER==1
        mFileTransferServicePtr.reset( new FileTransferService() );
        addService(mFileTransferServicePtr);
#endif
        
    }

    bool RcfServer::addService(ServicePtr servicePtr)
    {
        RCF_LOG_2()(typeid(*servicePtr).name()) << "RcfServer - adding service.";

        RCF_ASSERT(!mStarted && "Services cannot be added or removed while server is running.");

        if (
            std::find(
                mServices.begin(),
                mServices.end(),
                servicePtr) == mServices.end())
        {
            mServices.push_back(servicePtr);

            ServerTransportPtr serverTransportPtr =
                boost::dynamic_pointer_cast<ServerTransport>(servicePtr);

            if (serverTransportPtr)
            {
                mServerTransports.push_back(serverTransportPtr);
            }

            servicePtr->onServiceAdded(*this);

            return true;
        }
        return false;
    }

    bool RcfServer::removeService(ServicePtr servicePtr)
    {
        RCF_LOG_2()(typeid(*servicePtr).name()) << "Removing service.";

        RCF_ASSERT(!mStarted && "Services cannot be added or removed while server is running.");

        std::vector<ServicePtr>::iterator iter =
            std::find(mServices.begin(), mServices.end(), servicePtr);

        if (iter != mServices.end())
        {
            mServices.erase(iter);

            ServerTransportPtr serverTransportPtr =
                boost::dynamic_pointer_cast<ServerTransport>(servicePtr);

            if (serverTransportPtr)
            {
                eraseRemove(mServerTransports, serverTransportPtr);
            }

            stopService(servicePtr);
            servicePtr->onServiceRemoved(*this);
            return true;
        }
        return false;
    }

    bool RcfServer::addServerTransport(ServerTransportPtr serverTransportPtr)
    {
        return addService(
            boost::dynamic_pointer_cast<I_Service>(serverTransportPtr));
    }

    bool RcfServer::removeServerTransport(ServerTransportPtr serverTransportPtr)
    {
        return removeService(
            boost::dynamic_pointer_cast<I_Service>(serverTransportPtr));
    }

    void RcfServer::start()
    {
        startImpl();
    }

    void RcfServer::resolveServiceThreadPools(ServicePtr servicePtr) const
    {
        I_Service & service = *servicePtr;
        TaskEntries & taskEntries = service.mTaskEntries;
        for (std::size_t j=0; j<taskEntries.size(); ++j)
        {
            TaskEntry & taskEntry = taskEntries[j];

            // Muxer type == 0 , means we have to have a local thread manager.
            if (taskEntry.mMuxerType == Mt_None && !taskEntry.mLocalThreadPoolPtr)
            {
                taskEntry.mLocalThreadPoolPtr.reset( new ThreadPool(1) );

                if ( mThreadPoolPtr && boost::dynamic_pointer_cast<UdpServerTransport>(servicePtr) )
                {
                    for ( std::size_t i = 0; i < mThreadPoolPtr->mThreadInitFunctors.size(); ++i )
                    {
                        taskEntry.mLocalThreadPoolPtr->addThreadInitFunctor(mThreadPoolPtr->mThreadInitFunctors[i]);
                    }
                    for ( std::size_t i = 0; i < mThreadPoolPtr->mThreadDeinitFunctors.size(); ++i )
                    {
                        taskEntry.mLocalThreadPoolPtr->addThreadDeinitFunctor(mThreadPoolPtr->mThreadDeinitFunctors[i]);
                    }
                }
            }

            if (taskEntry.mLocalThreadPoolPtr)
            {
                taskEntry.mLocalThreadPoolPtr->setTask(taskEntry.mTask);
                taskEntry.mLocalThreadPoolPtr->setStopFunctor(taskEntry.mStopFunctor);
                taskEntry.mLocalThreadPoolPtr->setThreadName(taskEntry.mThreadName);
                taskEntry.mWhichThreadPoolPtr = taskEntry.mLocalThreadPoolPtr;
            }
            else if (service.mThreadPoolPtr)
            {
                taskEntry.mWhichThreadPoolPtr = service.mThreadPoolPtr;
            }
            else
            {
                taskEntry.mWhichThreadPoolPtr = mThreadPoolPtr;
            }

            taskEntry.mWhichThreadPoolPtr->enableMuxerType(taskEntry.mMuxerType);
        }
    }

    void RcfServer::startImpl()
    {
        Lock lock(mStartStopMutex);
        if (!mStarted)
        {
            // open the server

            for (std::size_t i=0; i<mServices.size(); ++i)
            {
                resolveServiceThreadPools(mServices[i]);
            }

            // notify all services
            for (std::size_t i=0; i<mServices.size(); ++i)
            {
                mServices[i]->onServerStart(*this);
            }

            // spawn internal worker threads
            for (std::size_t i=0; i<mServices.size(); ++i)
            {
                startService(mServices[i]);
            }

            mStarted = true;

            // call the start notification callback, if there is one
            invokeStartCallback();

            // notify anyone who was waiting on the stop event
            mStartEvent.notify_all(lock);
        }
    }

    void RcfServer::startService(ServicePtr servicePtr) const
    {
        RCF_LOG_2()(typeid(*servicePtr).name()) << "RcfServer - starting service.";

        TaskEntries &taskEntries = servicePtr->mTaskEntries;

        for (std::size_t i=0; i<taskEntries.size(); ++i)
        {
            if (taskEntries[i].getAutoStart())
            {
                taskEntries[i].start();
            }
        }
    }

    void RcfServer::stopService(ServicePtr servicePtr) const
    {
        RCF_LOG_2()(typeid(*servicePtr).name()) << "RcfServer - stopping service.";

        TaskEntries &taskEntries = servicePtr->mTaskEntries;

        for (std::size_t i=taskEntries.size()-1; i != std::size_t(-1); --i)
        {
            taskEntries[i].stop();
        }
    }

    void RcfServer::stop()
    {
        Lock lock(mStartStopMutex);
        if (mStarted)
        {
            mStarted = false;

            for (std::size_t i=mServices.size()-1; i != std::size_t(-1); --i)
            {
                stopService(mServices[i]);
            }

            // notify all services
            for (std::size_t i=mServices.size()-1; i != std::size_t(-1); --i)
            {
                mServices[i]->onServerStop(*this);
            }

            // Reset all muxers.
            if (mThreadPoolPtr)
            {
                mThreadPoolPtr->resetMuxers();
            }

            for (std::size_t i=mServices.size()-1; i != std::size_t(-1); --i)
            {
                mServices[i]->resetMuxers();
            }

            // notify anyone who was waiting on the stop event
            mStopEvent.notify_all(lock);
        }
    }

    void RcfServer::waitForStopEvent()
    {
        Lock lock(mStartStopMutex);
        if (mStarted)
        {
            mStopEvent.wait(lock);
        }
    }

    void RcfServer::waitForStartEvent()
    {
        Lock lock(mStartStopMutex);
        if (!mStarted)
        {
            mStartEvent.wait(lock);
        }
    }   

    bool RcfServer::isStarted()
    {
        return mStarted;
    }

    SessionPtr RcfServer::createSession()
    {
        RcfSessionPtr rcfSessionPtr(new RcfSession(*this));
        rcfSessionPtr->setWeakThisPtr();
        return rcfSessionPtr;
    }

    void RcfServer::onReadCompleted(SessionPtr sessionPtr)
    {
        RcfSessionPtr rcfSessionPtr = sessionPtr;

        // Need a recursion limiter here. When processing many sequential oneway
        // calls, over a caching transport filter such as the zlib filter, we 
        // would otherwise be at risk of encountering unlimited recursion and 
        // eventual stack overflow.

        RecursionState<int, int> & recursionState = 
            getTlsRcfSessionRecursionState();

        applyRecursionLimiter(
            recursionState, 
            &RcfSession::onReadCompleted, 
            *rcfSessionPtr);

        //rcfSessionPtr->onReadCompleted();
    }

    void RcfSession::onReadCompleted()
    {
        // 1. Deserialize request data
        // 2. Store request data in session
        // 3. Move session to corresponding queue

        Lock lock(mStopCallInProgressMutex);
        if (!mStopCallInProgress)
        {
            ServerTransport & transport = mpNetworkSession->getServerTransport();

            // If this is the first call on a Win32 pipe transport, impersonate the client
            // and get their username.

#if RCF_FEATURE_NAMEDPIPE==1

            if (    getTransportType() == Tt_Win32NamedPipe 
                &&  mClientUsername.empty() 
                &&  !getIsCallbackSession())
            {
                Win32NamedPipeImpersonator impersonator;
                tstring domainAndUsername = RCF::getMyDomain() + RCF_T("\\") + RCF::getMyUserName();
                getCurrentRcfSession().mClientUsername = domainAndUsername;
            }

#endif

            RpcProtocol rpcProtocol = transport.getRpcProtocol();
            if (rpcProtocol == Rp_JsonRpc)
            {
                processJsonRpcRequest();
                return;
            }

            ByteBuffer readByteBuffer = getNetworkSession().getReadByteBuffer();

            RCF_LOG_3()(this)(readByteBuffer.getLength()) 
                << "RcfServer - received packet from transport.";

            ByteBuffer messageBody;

            bool ok = mRequest.decodeRequest(
                readByteBuffer,
                messageBody,
                shared_from_this(),
                mRcfServer);

            RCF_LOG_3()(this)(mRequest) 
                << "RcfServer - received request.";

            // Setup the in stream for this remote call.
            mIn.reset(
                messageBody, 
                mRequest.mSerializationProtocol, 
                mRuntimeVersion, 
                mArchiveVersion,
                mRequest.mEnableSfPointerTracking);

            messageBody.clear();
            
            readByteBuffer.clear();

            if (!ok)
            {
                // version mismatch (client is newer than we are)
                // send a control message back to client, with our runtime version

                if (mRequest.mOneway)
                {
                    mIn.clearByteBuffer();
                    onWriteCompleted();
                }
                else
                {
                    std::vector<ByteBuffer> byteBuffers(1);

                    encodeServerError(
                        mRcfServer,
                        byteBuffers.front(),
                        RcfError_VersionMismatch, 
                        mRcfServer.getRuntimeVersion(),
                        mRcfServer.getArchiveVersion());

                    getNetworkSession().postWrite(byteBuffers);
                }
            }
            else
            {
                if (mRequest.getClose()) 
                {
                    getNetworkSession().postClose();
                }
                else
                {
                    processRequest();
                }
            }
        }
    }

    void RcfServer::onWriteCompleted(SessionPtr sessionPtr)
    {
        RcfSessionPtr rcfSessionPtr = sessionPtr;
        rcfSessionPtr->onWriteCompleted();
    }

    void RcfSession::onWriteCompleted()
    {
        RCF_LOG_3()(this) << "RcfServer - completed sending of response.";

        {
            Lock lock(mIoStateMutex);

            if (mWritingPingBack)
            {
                mWritingPingBack = false;

                typedef std::vector<ByteBuffer> ByteBuffers;
                ThreadLocalCached< ByteBuffers > tlcQueuedBuffers;
                ByteBuffers & queuedBuffers = tlcQueuedBuffers.get();

                queuedBuffers = mQueuedSendBuffers;
                mQueuedSendBuffers.clear();
                if (!queuedBuffers.empty())
                {
                    lock.unlock();
                    getNetworkSession().postWrite(queuedBuffers);
                    RCF_ASSERT(queuedBuffers.empty());
                }

                return;
            }
        }

        typedef std::vector<RcfSession::OnWriteCompletedCallback> OnWriteCompletedCallbacks;
        ThreadLocalCached< OnWriteCompletedCallbacks > tlcOwcc;
        OnWriteCompletedCallbacks &onWriteCompletedCallbacks = tlcOwcc.get();
        
        extractOnWriteCompletedCallbacks(onWriteCompletedCallbacks);

        for (std::size_t i=0; i<onWriteCompletedCallbacks.size(); ++i)
        {
            onWriteCompletedCallbacks[i](*this);
        }

        onWriteCompletedCallbacks.resize(0);

        mIn.clear();
        mOut.clear();

        if (!mCloseSessionAfterWrite)
        {
            getNetworkSession().postRead();
        }        
    }

    void RcfSession::sendSessionResponse()
    {
        mIn.clearByteBuffer();

        ThreadLocalCached< std::vector<ByteBuffer> > tlcByteBuffers;
        std::vector<ByteBuffer> &byteBuffers = tlcByteBuffers.get();

        mOut.extractByteBuffers(byteBuffers);
        const std::vector<FilterPtr> &filters = mFilters;
        ThreadLocalCached< std::vector<ByteBuffer> > tlcEncodedByteBuffers;
        std::vector<ByteBuffer> &encodedByteBuffers = tlcEncodedByteBuffers.get();

        ThreadLocalCached< std::vector<FilterPtr> > tlcNoFilters;
        std::vector<FilterPtr> &noFilters = tlcNoFilters.get();

        mRequest.encodeToMessage(
            encodedByteBuffers, 
            byteBuffers, 
            mFiltered ? filters : noFilters);

        RCF_LOG_3()(this)(lengthByteBuffers(byteBuffers))(lengthByteBuffers(encodedByteBuffers))
            << "RcfServer - sending response.";

        byteBuffers.resize(0);

        bool okToWrite = false;
        {
            Lock lock(mIoStateMutex);
            unregisterForPingBacks();
            if (mWritingPingBack)
            {
                mQueuedSendBuffers = encodedByteBuffers;
                encodedByteBuffers.resize(0);
                byteBuffers.resize(0);
            }
            else
            {
                okToWrite = true;
            }
        }

        if (okToWrite)
        {
            getNetworkSession().postWrite(encodedByteBuffers);
            RCF_ASSERT(encodedByteBuffers.empty());
            RCF_ASSERT(byteBuffers.empty());
        }

        setTlsRcfSessionPtr();
    }

    void RcfSession::sendResponseUncaughtException()
    {
        RCF_LOG_3() << "RcfServer - non-std::exception-derived exception was thrown. Sending an error response.";
        sendResponseException( RemoteException(_RcfError_NonStdException()));
    }

    void RcfSession::encodeRemoteException(
        SerializationProtocolOut & out, 
        const RemoteException & e)
    {
        ByteBuffer buffer;
        bool shouldSerializeException = mRequest.encodeResponse(&e, buffer, false);

        mOut.reset(
            mRequest.mSerializationProtocol, 
            32, 
            buffer, 
            mRuntimeVersion, 
            mArchiveVersion,
            mEnableSfPointerTracking);

        if (shouldSerializeException)
        {
            if (
                out.getSerializationProtocol() == Sp_BsBinary 
                || out.getSerializationProtocol() == Sp_BsText)
            {
                int runtimeVersion = mRequest.mRuntimeVersion;
                if (runtimeVersion < 8)
                {
                    // Boost serialization is very picky about serializing pointers 
                    // vs values. Since the client will deserialize an auto_ptr, we 
                    // are forced to create an auto_ptr here as well.

                    std::auto_ptr<RemoteException> apRe( 
                        static_cast<RemoteException *>(e.clone().release()) );

                    serialize(out, apRe);
                }
                else
                {
                    const RCF::RemoteException * pRe = &e;
                    serialize(out, pRe);
                }
            }
            else
            {
                // SF is a bit more flexible.
                serialize(out, e);
            }
        }
    }

    void RcfSession::sendResponse()
    {
        bool exceptionalResponse = false;
        try
        {
            ByteBuffer buffer;
            mRequest.encodeResponse(NULL, buffer, mEnableSfPointerTracking);

            mOut.reset(
                mRequest.mSerializationProtocol, 
                32, 
                buffer, 
                mRuntimeVersion, 
                mArchiveVersion,
                mEnableSfPointerTracking);

            mpParameters->write(mOut);
            clearParameters();
        }
        catch(const std::exception &e)
        {
            sendResponseException(e);
            exceptionalResponse = true;
        }

        if (!exceptionalResponse)
        {
            sendSessionResponse();
            RCF_LOG_2() << "RcfServer - end remote call. " << mCurrentCallDesc;
        }
    }

    void RcfSession::sendResponseException(
        const std::exception &e)
    {
        clearParameters();

        const SerializationException *pSE =
            dynamic_cast<const SerializationException *>(&e);

        const RemoteException *pRE =
            dynamic_cast<const RemoteException *>(&e);

        const Exception *pE =
            dynamic_cast<const Exception *>(&e);

        if (pSE)
        {
            RCF_LOG_1()(typeid(*pSE))(*pSE) << "Returning RCF::SerializationException to client.";
            encodeRemoteException(
                mOut,
                RemoteException(
                    Error( pSE->getError() ),
                    pSE->what(),
                    pSE->getContext(),
                    typeid(*pSE).name()));
        }
        else if (pRE)
        {
            RCF_LOG_1()(typeid(*pRE))(*pRE) << "Returning RCF::RemoteException to client.";
            try
            {
                encodeRemoteException(mOut, *pRE);
            }
            catch(const RCF::Exception &exc)
            {
                encodeRemoteException(
                    mOut,
                    RemoteException(
                        _RcfError_Serialization(typeid(*pRE).name(), typeid(exc).name(), exc.getError().getErrorString()),
                        exc.getWhat(),
                        exc.getContext(),
                        typeid(exc).name()));
            }
            catch(const std::exception &exc)
            {
                encodeRemoteException(
                    mOut,
                    RemoteException(
                        _RcfError_Serialization(typeid(*pRE).name(), typeid(exc).name(), exc.what()),
                        exc.what(),
                        "",
                        typeid(exc).name()));
            }
        }
        else if (pE)
        {
            RCF_LOG_1()(typeid(*pE))(*pE) << "Returning RCF::Exception to client.";
            encodeRemoteException(
                mOut,
                RemoteException(
                    Error( pE->getError() ),
                    pE->getSubSystemError(),
                    pE->getSubSystem(),
                    pE->what(),
                    pE->getContext(),
                    typeid(*pE).name()));
        }
        else
        {
            RCF_LOG_1()(typeid(e))(e) << "Returning std::exception to client.";
            encodeRemoteException(
                mOut,
                RemoteException(
                    _RcfError_AppException(typeid(e).name(), e.what()),
                    e.what(),
                    "",
                    typeid(e).name()));
        }

        sendSessionResponse();

        RCF_LOG_2() << "RcfServer - end remote call. " << mCurrentCallDesc;
    }

    class SessionTouch 
    {
    public:
        SessionTouch(RcfSession &rcfSession) : mRcfSession(rcfSession)
        {
            mRcfSession.touch();
        }
        ~SessionTouch()
        {
            mRcfSession.touch();
        }

    private:
        RcfSession & mRcfSession;
    };

    class StubEntryTouch 
    {
    public:
        StubEntryTouch(StubEntryPtr stubEntry) : mStubEntry(stubEntry)
        {
            if (mStubEntry)
            {
                mStubEntry->touch();
            }
        }
        ~StubEntryTouch()
        {
            if (mStubEntry)
            {
                mStubEntry->touch();
            }
        }

    private:
        StubEntryPtr mStubEntry;
    };

#if RCF_FEATURE_JSON==1

    void RcfSession::processJsonRpcRequest()
    {
        ByteBuffer readByteBuffer = getNetworkSession().getReadByteBuffer();

        RCF_LOG_3()(this)(readByteBuffer.getLength()) 
            << "RcfServer - received JSON-RPC packet from transport.";

        CurrentRcfSessionSentry guard(*this);

        boost::scoped_ptr<JsonRpcResponse> jsonResponsePtr;
        bool isOneway = false;
        boost::uint64_t jsonRequestId = 0;

        try
        {
            JsonRpcRequest jsonRequest(readByteBuffer);
            jsonRequestId = jsonRequest.getRequestId();
            jsonResponsePtr.reset( new JsonRpcResponse(jsonRequestId) );

            std::string jsonRpcName = jsonRequest.getMethodName();
            isOneway = jsonRequest.isNotification();

            RcfServer::JsonRpcMethod jsonRpcMethod;

            {
                ReadLock lock(mRcfServer.mStubMapMutex);
                RcfServer::JsonRpcMethods::iterator iter = mRcfServer.mJsonRpcMethods.find(jsonRpcName);
                if (iter != mRcfServer.mJsonRpcMethods.end())
                {
                    jsonRpcMethod = iter->second;
                }
            }

            if (jsonRpcMethod)
            {
                jsonRpcMethod(jsonRequest, *jsonResponsePtr);
            }
            else
            {
                std::string errMsg = "Unknown JSON-RPC method name: " + jsonRpcName;
                jsonResponsePtr->getJsonResponse()["error"] = errMsg;
            }
        }
        catch(...)
        {
            std::string errMsg;
            try
            {
                throw;
            }
            catch(const RCF::Exception & e)
            {
                errMsg = e.getErrorString();
            }
            catch(const std::exception & e)
            {
                errMsg = e.what();
            }
            catch(const std::string & jsonSpiritErrMsg)
            {
                errMsg = jsonSpiritErrMsg;
            }
            catch(...)
            {
                errMsg = "Caught C++ exception of unknown type.";
            }

            jsonResponsePtr.reset( new JsonRpcResponse(jsonRequestId) );
            jsonResponsePtr->getJsonResponse()["result"] = json_spirit::mValue();
            jsonResponsePtr->getJsonResponse()["error"] = errMsg;
        }

        setTlsRcfSessionPtr();

        if (isOneway)
        {
            onWriteCompleted();
        }
        else
        {
            json_spirit::mObject & obj = jsonResponsePtr->getJsonResponse();

            MemOstreamPtr osPtr = getObjectPool().getMemOstreamPtr();
            json_spirit::write_stream(json_spirit::mValue(obj), *osPtr, json_spirit::pretty_print);
            ByteBuffer buffer(osPtr->str(), static_cast<std::size_t>(osPtr->tellp()), osPtr);
            ThreadLocalCached< std::vector<ByteBuffer> > tlcByteBuffers;
            std::vector<ByteBuffer> & buffers = tlcByteBuffers.get();
            buffers.push_back(buffer);
            getNetworkSession().postWrite(buffers);
        }
    }

#else

    void RcfSession::processJsonRpcRequest()
    {
        std::string jsonErrorResponse = 
            "{\"error\" : \"RCF not built with JSON-RPC support.\", \"id\" : null}";

        ByteBuffer buffer(jsonErrorResponse);
        std::vector<ByteBuffer> buffers;
        buffers.push_back(buffer);
        getNetworkSession().postWrite(buffers);
    }

#endif


    void RcfSession::processRequest()
    {
        CurrentRcfSessionSentry guard(*this);

        using namespace boost::multi_index::detail;

        scope_guard sendResponseUncaughtExceptionGuard =
            make_obj_guard(
            *this,
            &RcfSession::sendResponseUncaughtException);

        try
        {
            mAutoSend = true;

            ++mRemoteCallCount;
        
            invokeServant();

            sendResponseUncaughtExceptionGuard.dismiss();

            // mAutoSend is false for async server dispatch.
            if (mAutoSend)
            {
                if (mRequest.mOneway)
                {
                    RCF_LOG_2()(this) << "RcfServer - suppressing response to oneway call.";
                    mIn.clearByteBuffer();
                    clearParameters();
                    setTlsRcfSessionPtr();
                    onWriteCompleted();
                }
                else
                {
                    sendResponse();
                }
            }
        }
        catch(const std::exception &e)
        {
            sendResponseUncaughtExceptionGuard.dismiss();
            if (mAutoSend)
            {
                if (mRequest.mOneway)
                {
                    mIn.clearByteBuffer();
                    clearParameters();
                    setTlsRcfSessionPtr();
                    onWriteCompleted();
                    
                }
                else
                {
                    sendResponseException(e);
                }
            }
        }
    }
    
    void RcfSession::verifyTransportProtocol(RCF::TransportProtocol protocol)
    {
        std::vector<TransportProtocol> protocols;

        // Transport protocols can be set on either the server transport or the server.
        protocols = mpNetworkSession->getServerTransport().getSupportedProtocols();
        if (protocols.empty())
        {
            protocols = mRcfServer.mSupportedProtocols;
        }

        if (protocols.size() > 0)
        {
            if (    std::find(protocols.begin(), protocols.end(), protocol) 
                ==  protocols.end())
            {
                // Protocol not supported. Put together an error message listing
                // the supported protocols.

                std::string protocolList = "[ ";
                for (std::size_t i=0; i<protocols.size(); ++i)
                {
                    if (i > 0)
                    {
                        protocolList += ", ";
                    }
                    protocolList += getTransportProtocolName(protocols[i]);
                }
                protocolList += " ]";

                RCF_THROW( Exception( _RcfError_ClearCommunicationNotAllowed(protocolList) ));
            }
        }
    }

#if RCF_FEATURE_SERVER==1

    void RcfSession::processOobMessages()
    {
        if (mRequest.mOutOfBandRequest.getLength() > 0)
        {

            ThreadInfoPtr threadInfoPtr = getTlsThreadInfoPtr();
            if (threadInfoPtr)
            {
                threadInfoPtr->notifyBusy();
            }

            OobMessagePtr msgPtr = OobMessage::decodeRequestCommon(
                mRequest.mOutOfBandRequest);

            switch ( msgPtr->getMessageType() )
            {
            case Omt_RequestTransportFilters:
                {
                    OobRequestTransportFilters & rtfMsg = 
                        static_cast<OobRequestTransportFilters &>(*msgPtr);


#if RCF_FEATURE_SERVER==1
                    rtfMsg.mResponseError = 
                        mRcfServer.mFilterServicePtr->RequestTransportFilters(
                        rtfMsg.mFilterIds);
#else
                    Exception e(_RcfError_NotSupportedInThisBuild("Transport filters"));
                    rtfMsg.mResponseError = e.getErrorId();
                    rtfMsg.mResponseErrorString = e.getErrorString();
#endif

                }
                break;

            case Omt_CreateCallbackConnection:
                {
                    OobCreateCallbackConnection & rtfMsg = 
                        static_cast<OobCreateCallbackConnection &>(*msgPtr);

#if RCF_FEATURE_SERVER==1
                    mRcfServer.mCallbackConnectionServicePtr->CreateCallbackConnection();
                    rtfMsg.mResponseError = RcfError_Ok;
#else
                    Exception e(_RcfError_NotSupportedInThisBuild("Callback connections"));
                    rtfMsg.mResponseError = e.getErrorId();
                    rtfMsg.mResponseErrorString = e.getErrorString();
#endif
                }
                break;

            case Omt_RequestSubscription:
                {
                    OobRequestSubscription & rtfMsg = 
                        static_cast<OobRequestSubscription &>(*msgPtr);

#if RCF_FEATURE_PUBSUB==1
                    rtfMsg.mResponseError = mRcfServer.mPublishingServicePtr->RequestSubscription(
                        rtfMsg.mPublisherName, 
                        rtfMsg.mSubToPubPingIntervalMs, 
                        rtfMsg.mPubToSubPingIntervalMs);
#else
                    Exception e(_RcfError_NotSupportedInThisBuild("Subscriptions"));
                    rtfMsg.mResponseError = e.getErrorId();
                    rtfMsg.mResponseErrorString = e.getErrorString();
#endif
                }
                break;

            default:

                RCF_THROW( Exception(_RcfError_Decoding()) );
            }

            ByteBuffer buffer;
            msgPtr->encodeResponse(buffer);
            mRequest.mOutOfBandResponse = buffer;
        }
        else
        {
            mRequest.mOutOfBandResponse = ByteBuffer();
        }
    }

#else

    void RcfSession::processOobMessages()
    {
    }

#endif

    void RcfSession::invokeServant()
    {
        StubEntryPtr stubEntryPtr = mRequest.locateStubEntryPtr(mRcfServer);

        if (    NULL == stubEntryPtr.get() 
            &&  mRequest.getFnId() != -1)
        {
            Exception e( _RcfError_NoServerStub(
                mRequest.getService(), 
                mRequest.getSubInterface(),
                mRequest.getFnId()));

            RCF_THROW(e)(mRequest.getFnId());
        }
        else
        {
            setCachedStubEntryPtr(stubEntryPtr);

            SessionTouch sessionTouch(*this);

            StubEntryTouch stubEntryTouch(stubEntryPtr);

            if (!mTransportProtocolVerified)
            {
                bool bypassTransportProtocolCheck = 
                        (mRequest.getService() == "I_RequestTransportFilters")
                    ||  mRequest.mFnId == -1;

                bool doTransportProtocolCheck = ! bypassTransportProtocolCheck;

                if (doTransportProtocolCheck)
                {
                    verifyTransportProtocol(mTransportProtocol);
                    mTransportProtocolVerified = true;
                }
            }

            processOobMessages();

            if (mRequest.getFnId() == -1)
            {
                // Function id -1 is a canned ping request. Set a timestamp 
                // on the current session and return immediately.

                AllocateServerParameters<Void>()(*this);
                setPingTimestamp();
            }
            else
            {
                registerForPingBacks();

                ThreadInfoPtr threadInfoPtr = getTlsThreadInfoPtr();
                if (threadInfoPtr)
                {
                    threadInfoPtr->notifyBusy();
                }

                stubEntryPtr->getRcfClientPtr()->getServerStub().invoke(
                    mRequest.getSubInterface(),
                    mRequest.getFnId(),
                    *this);
            }
        }
    }

    ServerTransport &RcfServer::getServerTransport()
    {
        return *getServerTransportPtr();
    }

    I_Service &RcfServer::getServerTransportService()
    {
        return dynamic_cast<I_Service &>(*getServerTransportPtr());
    }

    ServerTransportPtr RcfServer::getServerTransportPtr()
    {
        RCF_ASSERT( ! mServerTransports.empty() );
        return mServerTransports[0];
    }

    IpServerTransport &RcfServer::getIpServerTransport()
    {
        return dynamic_cast<RCF::IpServerTransport &>(getServerTransport());
    }    

    ServerBindingPtr RcfServer::bindImpl(
        const std::string &name,
        RcfClientPtr rcfClientPtr)
    {
        RCF_ASSERT(rcfClientPtr.get());
        RCF_LOG_2()(name) << "RcfServer - exposing static binding.";

        WriteLock writeLock(mStubMapMutex);
        mStubMap[name] = StubEntryPtr( new StubEntry(rcfClientPtr));
        return rcfClientPtr->getServerStubPtr();
    }

#if RCF_FEATURE_JSON==1

    void RcfServer::bindJsonRpc(JsonRpcMethod jsonRpcMethod, const std::string & jsonRpcName)
    {
        RCF_ASSERT(!jsonRpcName.empty());

        WriteLock writeLock(mStubMapMutex);
        mJsonRpcMethods[jsonRpcName] = jsonRpcMethod;
    }

    void RcfServer::unbindJsonRpc(const std::string & jsonRpcName)
    {
        WriteLock writeLock(mStubMapMutex);

        JsonRpcMethods::iterator iter = mJsonRpcMethods.find(jsonRpcName);
        if (iter != mJsonRpcMethods.end())
        {
            mJsonRpcMethods.erase(iter);
        }
    }

#endif

    FilterPtr RcfServer::createFilter(int filterId)
    {
        if (mFilterServicePtr)
        {
            FilterFactoryPtr filterFactoryPtr = 
                mFilterServicePtr->getFilterFactoryPtr(filterId);

            if (filterFactoryPtr)
            {
                return filterFactoryPtr->createFilter(*this);
            }
        }

        RCF_THROW( Exception( _RcfError_UnknownFilter()) );
        return FilterPtr();
    }

    void RcfServer::setStartCallback(const StartCallback &startCallback)
    {
        mStartCallback = startCallback;
    }

    void RcfServer::invokeStartCallback()
    {
        if (mStartCallback)
        {
            mStartCallback(*this);
        }
    }

    boost::uint32_t RcfServer::getRuntimeVersion()
    {
        return mRuntimeVersion;
    }

    void RcfServer::setRuntimeVersion(boost::uint32_t version)
    {
        mRuntimeVersion = version;
    }

    boost::uint32_t RcfServer::getArchiveVersion()
    {
        return mArchiveVersion;
    }

    void RcfServer::setArchiveVersion(boost::uint32_t version)
    {
        mArchiveVersion = version;
    }

    PingBackServicePtr RcfServer::getPingBackServicePtr()
    {
        return mPingBackServicePtr;
    }

    FileTransferServicePtr RcfServer::getFileTransferServicePtr()
    {
        return mFileTransferServicePtr;
    }

    SessionTimeoutServicePtr RcfServer::getSessionTimeoutServicePtr()
    {
        return mSessionTimeoutServicePtr;
    }

    ObjectFactoryServicePtr RcfServer::getObjectFactoryServicePtr()
    {
        return mObjectFactoryServicePtr;
    }

    SessionObjectFactoryServicePtr RcfServer::getSessionObjectFactoryServicePtr()
    {
        return mSessionObjectFactoryServicePtr;
    }

    PublishingServicePtr RcfServer::getPublishingServicePtr()
    {
        return mPublishingServicePtr;
    }

    SubscriptionServicePtr RcfServer::getSubscriptionServicePtr()
    {
        return mSubscriptionServicePtr;
    }

    FilterServicePtr RcfServer::getFilterServicePtr() 
    {
        return mFilterServicePtr;
    }

    void RcfServer::setThreadPool(ThreadPoolPtr threadPoolPtr)
    {
        if (threadPoolPtr->getThreadName().empty())
        {
            threadPoolPtr->setThreadName("RCF Server");
        }
        mThreadPoolPtr = threadPoolPtr;
    }

    ThreadPoolPtr RcfServer::getThreadPool()
    {
        return mThreadPoolPtr;
    }

    ServerTransport & RcfServer::addEndpoint(const RCF::Endpoint & endpoint)
    {
        ServerTransportPtr transportPtr(endpoint.createServerTransport().release());
        addServerTransport(transportPtr);
        return *transportPtr;
    }

    ServerTransport & RcfServer::findTransportCompatibleWith(
        ClientTransport & clientTransport)
    {
        TransportType clientTransportType = clientTransport.getTransportType();
        for ( std::size_t i = 0; i < mServerTransports.size(); ++i )
        {
            ServerTransport& serverTransport = * mServerTransports[i];
            RCF::TransportType serverTransportType = serverTransport.getTransportType();
            if ( clientTransportType == serverTransportType )
            {
                return serverTransport;
            }
            else if (   clientTransportType == Tt_Http
                    ||  clientTransportType == Tt_Https )
            {
                if ( serverTransportType == Tt_Tcp )
                {
                    return serverTransport;
                }
            }
        }

        RCF_THROW(Exception("No corresponding server transport."));

        return *(ServerTransport *)NULL;
    }

    void RcfServer::setSupportedTransportProtocols(
        const std::vector<TransportProtocol> & protocols)
    {
        RCF_ASSERT(!mStarted);
        mSupportedProtocols = protocols;
    }

    const std::vector<TransportProtocol> & 
        RcfServer::getSupportedTransportProtocols() const
    {
        return mSupportedProtocols;
    }

    void RcfServer::setCertificate(CertificatePtr certificatePtr)
    {
        WriteLock lock(mPropertiesMutex);
        mCertificatePtr = certificatePtr;
    }

    CertificatePtr RcfServer::getCertificate()
    {
        ReadLock lock(mPropertiesMutex);
        return mCertificatePtr;
    }

    void RcfServer::setOpenSslCipherSuite(const std::string & cipherSuite)
    {
        WriteLock lock(mPropertiesMutex);
        mOpenSslCipherSuite = cipherSuite;
    }

    std::string RcfServer::getOpenSslCipherSuite() const
    {
        ReadLock lock(mPropertiesMutex);
        return mOpenSslCipherSuite;
    }



    //--------------------------------------------------------------------------
    // Certificate validation.

    void RcfServer::setCaCertificate(CertificatePtr caCertificatePtr)
    {
        WriteLock lock(mPropertiesMutex);
        mCaCertificatePtr = caCertificatePtr;

        mCertificateValidationCb.clear();
        mSchannelCertificateValidation.clear();
    }

    CertificatePtr RcfServer::getCaCertificate()
    {
        ReadLock lock(mPropertiesMutex);
        return mCaCertificatePtr;
    }

    void RcfServer::setCertificateValidationCallback(
        CertificateValidationCb certificateValidationCb)
    {
        WriteLock lock(mPropertiesMutex);
        mCertificateValidationCb = certificateValidationCb;

        mCaCertificatePtr.reset();
        mSchannelCertificateValidation.clear();
    }

    const RcfServer::CertificateValidationCb & RcfServer::getCertificateValidationCallback() const
    {
        ReadLock lock(mPropertiesMutex);
        return mCertificateValidationCb;
    }

    void RcfServer::setEnableSchannelCertificateValidation(const tstring & peerName)
    {
        WriteLock lock(mPropertiesMutex);
        mSchannelCertificateValidation = peerName;

        mCaCertificatePtr.reset();
        mCertificateValidationCb.clear();
    }

    tstring RcfServer::getEnableSchannelCertificateValidation() const
    {
        ReadLock lock(mPropertiesMutex);
        return mSchannelCertificateValidation;
    }

    void RcfServer::setSslImplementation(SslImplementation sslImplementation)
    {
        RCF_ASSERT(!mStarted);
        mSslImplementation = sslImplementation;
    }

    SslImplementation RcfServer::getSslImplementation() const
    {
        return mSslImplementation;
    }

    void RcfServer::setSessionTimeoutMs(boost::uint32_t sessionTimeoutMs)
    {
        RCF_ASSERT(!mStarted);
        mSessionTimeoutMs = sessionTimeoutMs;
    }

    boost::uint32_t RcfServer::getSessionTimeoutMs()
    {
        return mSessionTimeoutMs;
    }

    void RcfServer::setSessionHarvestingIntervalMs(boost::uint32_t sessionHarvestingIntervalMs)
    {
        RCF_ASSERT(!mStarted);
        mSessionHarvestingIntervalMs = sessionHarvestingIntervalMs;
    }

    boost::uint32_t RcfServer::getSessionHarvestingIntervalMs()
    {
        return mSessionHarvestingIntervalMs;
    }

    void RcfServer::setHttpSessionTimeoutMs(boost::uint32_t httpSessionTimeoutMs)
    {
        RCF_ASSERT(!mStarted);
        mHttpSessionTimeoutMs = httpSessionTimeoutMs;
    }

    boost::uint32_t RcfServer::getHttpSessionTimeoutMs()
    {
        return mHttpSessionTimeoutMs;
    }

    void RcfServer::setOnCallbackConnectionCreated(OnCallbackConnectionCreated onCallbackConnectionCreated)
    {
        RCF_ASSERT(!mStarted);
        mOnCallbackConnectionCreated = onCallbackConnectionCreated;
    }

    OnCallbackConnectionCreated RcfServer::getOnCallbackConnectionCreated()
    {
        return mOnCallbackConnectionCreated;
    }

    void RcfServer::setOfsMaxNumberOfObjects(boost::uint32_t ofsMaxNumberOfObjects)
    {
        RCF_ASSERT(!mStarted);
        mOfsMaxNumberOfObjects = ofsMaxNumberOfObjects;
    }

    void RcfServer::setOfsObjectTimeoutS(boost::uint32_t ofsObjectTimeoutS)
    {
        RCF_ASSERT(!mStarted);
        mOfsObjectTimeoutS = ofsObjectTimeoutS;
    }

    void RcfServer::setOfsCleanupIntervalS(boost::uint32_t ofsCleanupIntervalS)
    {
        RCF_ASSERT(!mStarted);
        mOfsCleanupIntervalS = ofsCleanupIntervalS;
    }

    void RcfServer::setOfsCleanupThreshold(float ofsCleanupThreshold)
    {
        RCF_ASSERT(!mStarted);
        mOfsCleanupThreshold = ofsCleanupThreshold;
    }

    boost::uint32_t RcfServer::getOfsMaxNumberOfObjects() const
    {
        return mOfsMaxNumberOfObjects;
    }

    boost::uint32_t  RcfServer::getOfsObjectTimeoutS() const
    {
        return mOfsObjectTimeoutS;
    }

    boost::uint32_t RcfServer::getOfsCleanupIntervalS() const
    {
        return mOfsCleanupIntervalS;
    }

    float RcfServer::getOfsCleanupThreshold() const
    {
        return mOfsCleanupThreshold;
    }

#if RCF_FEATURE_FILETRANSFER==1

    void RcfServer::setOnFileDownloadProgress(OnFileDownloadProgress onFileDownloadProgress)
    {
        RCF_ASSERT(!mStarted);
        mOnFileDownloadProgress = onFileDownloadProgress;
    }

    void RcfServer::setOnFileUploadProgress(OnFileUploadProgress onFileUploadProgress)
    {
        RCF_ASSERT(!mStarted);
        mOnFileUploadProgress = onFileUploadProgress;
    }

    void RcfServer::setFileUploadDirectory(const std::string & uploadFolder)
    {
        RCF_ASSERT(!mStarted);
        mFileUploadDirectory = uploadFolder;
    }

    std::string RcfServer::getFileUploadDirectory() const
    {
        return mFileUploadDirectory;
    }

    void RcfServer::setFileUploadBandwidthLimit(boost::uint32_t uploadQuotaBps)
    {
        RCF_ASSERT(!mStarted);
        mFileUploadQuota = uploadQuotaBps;
    }

    boost::uint32_t RcfServer::getFileUploadBandwidthLimit() const
    {
        return mFileUploadQuota;
    }

    void RcfServer::setFileUploadCustomBandwidthLimit(FileUploadQuotaCallback uploadQuotaCb)
    {
        RCF_ASSERT(!mStarted);
        mFileUploadQuotaCb = uploadQuotaCb;
    }

    void RcfServer::setFileDownloadBandwidthLimit(boost::uint32_t downloadQuotaBps)
    {
        RCF_ASSERT(!mStarted);
        mFileDownloadQuota = downloadQuotaBps;
    }

    boost::uint32_t RcfServer::getFileDownloadBandwidthLimit() const
    {
        return mFileDownloadQuota;
    }

    void RcfServer::setFileDownloadCustomBandwidthLimit(FileDownloadQuotaCallback downloadQuotaCb)
    {
        RCF_ASSERT(!mStarted);
        mFileDownloadQuotaCb = downloadQuotaCb;
    }

#endif

    boost::uint32_t RcfServer::getServerObjectHarvestingIntervalS() const
    {
        return mServerObjectHarvestingIntervalS;
    }

    void RcfServer::setServerObjectHarvestingIntervalS(boost::uint32_t harvestingIntervalS)
    {
        mServerObjectHarvestingIntervalS = harvestingIntervalS;
    }

    void RcfServer::deleteServerObject(const std::string & objectKey)
    {

#if RCF_FEATURE_SERVER==1
        return mServerObjectServicePtr->deleteServerObject(objectKey);
#else
        RCF_ASSERT(0 && "This RCF build does not support server objects.");
#endif

    }

    HttpSessionPtr RcfServer::attachHttpSession(
        const std::string &     httpSessionId, 
        bool                    allowCreate,
        ExceptionPtr &          ePtr)
    {
        HttpSessionPtr httpSessionPtr;
        {
            Lock lock(mHttpSessionMapMutex);
            
            std::map<std::string, HttpSessionPtr>::iterator iter =
                mHttpSessionMap.find(httpSessionId);

            if ( iter == mHttpSessionMap.end() )
            {
                
                if ( !allowCreate )
                {
                    ePtr.reset( new Exception(_RcfError_HttpSessionTimeout()) );
                    return HttpSessionPtr();
                }

                httpSessionPtr.reset(new HttpSession(httpSessionId));
                httpSessionPtr->mRcfSessionPtr = createSession();
                mHttpSessionMap[httpSessionId] = httpSessionPtr;
            }
            else
            {
                httpSessionPtr = iter->second;
            }
            RCF_ASSERT(!httpSessionPtr->mRequestInProgress);
            httpSessionPtr->mRequestInProgress = true;
            httpSessionPtr->mLastTouchMs = getCurrentTimeMs();
        }
        return httpSessionPtr;
    }

    void RcfServer::detachHttpSession(HttpSessionPtr httpSessionPtr)
    {
        Lock lock(mHttpSessionMapMutex);
        RCF_ASSERT(httpSessionPtr->mRequestInProgress);
        httpSessionPtr->mRequestInProgress = false;
        httpSessionPtr->mLastTouchMs = getCurrentTimeMs();

//#ifndef NDEBUG
//      Lock lock(mHttpSessionMapMutex);
//      std::map<std::string, HttpSessionPtr>::iterator iter = mHttpSessionMap.find(httpSessionPtr->mHttpSessionId);
//      RCF_ASSERT(iter != mHttpSessionMap.end());
//      RCF_ASSERT(iter != mHttpSessionMap.end() && iter->second == httpSessionPtr);
//#endif

    }

    void RcfServer::harvestHttpSessions()
    {
        boost::uint32_t nowMs = getCurrentTimeMs();

        Lock lock(mHttpSessionMapMutex);
        
        RCF_LOG_3()(mHttpSessionMap.size()) << "RcfServer::harvestHttpSessions() - entry.";

        std::map<std::string, HttpSessionPtr>::iterator iter = mHttpSessionMap.begin();
        while (iter != mHttpSessionMap.end())
        {
            HttpSessionPtr httpSessionPtr = iter->second;
            boost::uint32_t idleDurationMs = nowMs - httpSessionPtr->mLastTouchMs;
            if (    !httpSessionPtr->mRequestInProgress 
                &&  idleDurationMs > mHttpSessionTimeoutMs )
            {
                RCF_LOG_3()(idleDurationMs)(httpSessionPtr->mHttpSessionId)(httpSessionPtr->mHttpSessionId)(httpSessionPtr->mHttpSessionIndex)
                    << "RcfServer::harvestHttpSessions() - destroying HTTP session.";

                mHttpSessionMap.erase(iter++);
            }
            else
            {
                iter++;
            }
        }

        RCF_LOG_3()(mHttpSessionMap.size()) << "RcfServer::harvestHttpSessions() - exit.";
    }

}
