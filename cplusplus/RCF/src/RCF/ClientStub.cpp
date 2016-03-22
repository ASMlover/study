
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

#include <RCF/ClientStub.hpp>

#include <boost/bind.hpp>

#include <RCF/AmiThreadPool.hpp>
#include <RCF/AsioServerTransport.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/FileIoThreadPool.hpp>
#include <RCF/Future.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/IpClientTransport.hpp>
#include <RCF/Marshal.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/Version.hpp>

namespace RCF {

    //****************************************
    // ClientStub

#if defined(BOOST_WINDOWS) && RCF_FEATURE_OPENSSL==0

    // Windows builds without OpenSSL.
    SslImplementation gDefaultSslImplementation = Si_Schannel;

#else

    // All other builds.
    SslImplementation gDefaultSslImplementation = Si_OpenSsl;

#endif

    void setDefaultSslImplementation(SslImplementation sslImplementation)
    {
        gDefaultSslImplementation = sslImplementation;
    }

    SslImplementation getDefaultSslImplementation()
    {
        return gDefaultSslImplementation;
    }

    // 2s default connect timeout
    static unsigned int gClientConnectTimeoutMs = 1000*2;

    // 10s default call timeout
    static unsigned int gClientRemoteCallTimeoutMs = 1000*10;
    
    void setDefaultConnectTimeoutMs(unsigned int connectTimeoutMs)
    {
        gClientConnectTimeoutMs = connectTimeoutMs;
    }

    unsigned int getDefaultConnectTimeoutMs()
    {
        return gClientConnectTimeoutMs;
    }

    void setDefaultRemoteCallTimeoutMs(unsigned int remoteCallTimeoutMs)
    {
        gClientRemoteCallTimeoutMs = remoteCallTimeoutMs;
    }

    unsigned int getDefaultRemoteCallTimeoutMs()
    {
        return gClientRemoteCallTimeoutMs;
    }

    // Default wstring serialization is UTF-8 encoded.
    static bool gUseNativeWstringSerialization = false;

    void setDefaultNativeWstringSerialization(bool enable)
    {
        gUseNativeWstringSerialization = enable;
    }

    bool getDefaultNativeWstringSerialization()
    {
        return gUseNativeWstringSerialization;
    }

    void ClientStub::setAutoReconnect(bool autoReconnect)
    {
        mAutoReconnect = autoReconnect;
    }

    bool ClientStub::getAutoReconnect() const
    {
        return mAutoReconnect;
    }

    void ClientStub::setClientProgressPtr(ClientProgressPtr ClientProgressPtr)
    {
        mClientProgressPtr = ClientProgressPtr;
    }

    ClientProgressPtr ClientStub::getClientProgressPtr() const
    {
        return mClientProgressPtr;
    }

    void ClientStub::setRemoteCallProgressCallback(
        RemoteCallProgressCallback cb, 
        boost::uint32_t callbackIntervalMs)
    {
        ClientProgressPtr progressPtr( new ClientProgress() );
        progressPtr->mTimerIntervalMs = callbackIntervalMs;
        progressPtr->mTriggerMask = ClientProgress::Timer;
        progressPtr->mProgressCallback = cb;

        setClientProgressPtr(progressPtr);
    }

    static const boost::uint32_t DefaultBatchMaxMessageLimit = 1024*1024;

    ClientStub::ClientStub(const std::string &interfaceName)
    {
        init(interfaceName, "");
    }

    ClientStub::ClientStub(const std::string &interfaceName, const std::string &objectName)
    {
        init(interfaceName, objectName);
    }

    ClientStub::ClientStub(const ClientStub &rhs)
    {
        init(rhs.mInterfaceName, rhs.mObjectName);
        assign(rhs);
    }

    void ClientStub::init(const std::string & interfaceName, const std::string & objectName)
    {
        mDefaultCallingSemantics        = Twoway;
        mProtocol                       = DefaultSerializationProtocol;
        mMarshalingProtocol             = DefaultMarshalingProtocol;
        mObjectName                     = objectName;
        mInterfaceName                  = interfaceName;
        mRemoteCallTimeoutMs            = gClientRemoteCallTimeoutMs;
        mConnectTimeoutMs               = gClientConnectTimeoutMs;
        mAutoReconnect                  = true;
        mConnected                      = false;
        mTries                          = 0;
        mAutoVersioning                 = true;
        mRuntimeVersion                 = RCF::getDefaultRuntimeVersion();
        mArchiveVersion                 = RCF::getDefaultArchiveVersion();
        mUseNativeWstringSerialization  = RCF::getDefaultNativeWstringSerialization();
        mEnableSfPointerTracking        = false;

        mAsync                          = false;
        mAsyncOpType                    = None;
        mEndTimeMs                      = 0;
        mRetry                          = false;
        mRcs                            = Twoway;
        mpParameters                    = NULL;
        mPingBackIntervalMs             = 0;
        mPingBackTimeStamp              = 0;
        mPingBackCount                  = 0;
        mNextTimerCallbackMs            = 0;
        mNextPingBackCheckMs            = 0;
        mPingBackCheckIntervalMs        = 0;
        mTimerIntervalMs                = 0;

        mSignalled                      = false;

        mBatchMode                      = false;
        mBatchMaxMessageLength          = DefaultBatchMaxMessageLimit;
        mBatchCount                     = 0;
        mBatchMessageCount              = 0;
        mSetTransportProtocol           = false;

        mTransferWindowS                = 5;
        mCallInProgress                 = false;

        mHttpProxyPort                  = 0;
        mTransportProtocol              = Tp_Clear;
        mEnableCompression              = false;

        mSslImplementation              = RCF::getDefaultSslImplementation();

    }

    void ClientStub::assign(const ClientStub & rhs)
    {
        if (&rhs != this)
        {
            mInterfaceName                  = rhs.mInterfaceName;
            mToken                          = rhs.mToken;
            mDefaultCallingSemantics        = rhs.mDefaultCallingSemantics;
            mProtocol                       = rhs.mProtocol;
            mMarshalingProtocol             = rhs.mMarshalingProtocol;
            mEndpointName                   = rhs.mEndpointName;
            mObjectName                     = rhs.mObjectName;
            mRemoteCallTimeoutMs            = rhs.mRemoteCallTimeoutMs;
            mConnectTimeoutMs               = rhs.mConnectTimeoutMs;
            mAutoReconnect                  = rhs.mAutoReconnect;
            mConnected                      = false;
            mAutoVersioning                 = rhs.mAutoVersioning;
            mRuntimeVersion                 = rhs.mRuntimeVersion;
            mArchiveVersion                 = rhs.mArchiveVersion;
            mUseNativeWstringSerialization  = rhs.mUseNativeWstringSerialization;
            mEnableSfPointerTracking        = rhs.mEnableSfPointerTracking;
            mUserData                       = rhs.mUserData;
            mPingBackIntervalMs             = rhs.mPingBackIntervalMs;
            mSignalled                      = false;

            setEndpoint(rhs.getEndpoint());

            mClientProgressPtr              = rhs.mClientProgressPtr;

#if RCF_FEATURE_FILETRANSFER==1
            mFileProgressCb                 = rhs.mFileProgressCb;
            mTransferWindowS                = rhs.mTransferWindowS;
#endif

            mHttpProxy                      = rhs.mHttpProxy;
            mHttpProxyPort                  = rhs.mHttpProxyPort;
            mTransportProtocol              = rhs.mTransportProtocol;
            mUsername                       = rhs.mUsername;
            mPassword                       = rhs.mPassword;
            mKerberosSpn                    = rhs.mKerberosSpn;
            mEnableCompression              = rhs.mEnableCompression;

            mCertificatePtr                 = rhs.mCertificatePtr;
            mCaCertificatePtr               = rhs.mCaCertificatePtr;
            mCertificateValidationCb        = rhs.mCertificateValidationCb;
            mSchannelCertificateValidation  = rhs.mSchannelCertificateValidation;
            mOpenSslCipherSuite             = rhs.mOpenSslCipherSuite;

            mSslImplementation              = rhs.mSslImplementation;
        }
    }

    ClientStub::~ClientStub()
    {
        disconnect();
        clearParameters();        
    }

    void ClientStub::clearParameters()
    {
        if (mpParameters)
        {
            CurrentClientStubSentry sentry(*this);
            mpParameters->~I_Parameters();
            mpParameters = NULL;
        }
    }

    ClientStub &ClientStub::operator=( const ClientStub &rhs )
    {
        assign(rhs);
        return *this;
    }

    Token ClientStub::getTargetToken() const
    {
        return mToken;
    }

    void ClientStub::setTargetToken(Token token)
    {
        mToken = token;
    }

    const std::string & ClientStub::getTargetName() const
    {
        return mObjectName;
    }

    void ClientStub::setTargetName(const std::string &objectName)
    {
        mObjectName = objectName;
    }

    RemoteCallSemantics ClientStub::getRemoteCallSemantics() const
    {
        return mDefaultCallingSemantics;
    }

    void ClientStub::setRemoteCallSemantics(
        RemoteCallSemantics defaultCallingSemantics)
    {
        mDefaultCallingSemantics = defaultCallingSemantics;
    }

    RemoteCallSemantics ClientStub::getDefaultCallingSemantics() const
    {
        return getRemoteCallSemantics();
    }

    void ClientStub::setDefaultCallingSemantics(RemoteCallSemantics defaultCallingSemantics)
    {
        setRemoteCallSemantics(defaultCallingSemantics);
    }

    void ClientStub::setSerializationProtocol(SerializationProtocol  protocol)
    {
        mProtocol = protocol;
    }

    SerializationProtocol ClientStub::getSerializationProtocol() const
    {
        return mProtocol;
    }

    void ClientStub::setMarshalingProtocol(MarshalingProtocol  protocol)
    {
        mMarshalingProtocol = protocol;
    }

    MarshalingProtocol ClientStub::getMarshalingProtocol() const
    {
        return mMarshalingProtocol;
    }

    bool ClientStub::getNativeWstringSerialization()    
    {
        return mUseNativeWstringSerialization;
    }

    void ClientStub::setNativeWstringSerialization(bool useNativeWstringSerialization)
    {
        mUseNativeWstringSerialization = useNativeWstringSerialization;
    }

    void ClientStub::setEnableSfPointerTracking(bool enable)
    {
        mEnableSfPointerTracking = enable;
    }

    bool ClientStub::getEnableSfPointerTracking() const
    {
        return mEnableSfPointerTracking;
    }

    void ClientStub::setEndpoint(const Endpoint &endpoint)
    {
        mEndpoint = endpoint.clone();
    }

    void ClientStub::setEndpoint(EndpointPtr endpointPtr)
    {
        mEndpoint = endpointPtr;
    }

    EndpointPtr ClientStub::getEndpoint() const
    {
        return mEndpoint;
    }

    void ClientStub::setTransport(std::auto_ptr<ClientTransport> transport)
    {
        mTransport = transport;
        mConnected = mTransport.get() && mTransport->isConnected();
    }

    std::auto_ptr<ClientTransport> ClientStub::releaseTransport()
    {
        instantiateTransport();
        return mTransport;
    }

    ClientTransport& ClientStub::getTransport()
    {
        instantiateTransport();
        return *mTransport;
    }

    IpClientTransport &ClientStub::getIpTransport()
    {
        return dynamic_cast<IpClientTransport &>(getTransport());
    }

    void ClientStub::instantiateTransport()
    {
        CurrentClientStubSentry sentry(*this);
        if (!mTransport.get())
        {
            RCF_VERIFY(mEndpoint.get(), Exception(_RcfError_NoEndpoint()));
            mTransport.reset( mEndpoint->createClientTransport().release() );
            RCF_VERIFY(mTransport.get(), Exception(_RcfError_TransportCreation()));
        }

        if (    mAsync 
            &&  !mTransport->isAssociatedWithIoService())
        {
            RcfServer * preferred = getAsyncDispatcher();
            AsioIoService * pIoService = NULL;

            if (preferred)
            {
                ServerTransport & transport = preferred->getServerTransport();
                AsioServerTransport & asioTransport = dynamic_cast<AsioServerTransport &>(transport);
                pIoService = & asioTransport.getIoService();
            }
            else
            {
                pIoService = & getAmiThreadPool().getIoService();
            }

            mTransport->associateWithIoService(*pIoService);
        }
    }

    void ClientStub::disconnect()
    {
        std::string endpoint;
        if (mEndpoint.get())
        {
            endpoint = mEndpoint->asString();
        }

        RcfClientPtr subRcfClientPtr = getSubRcfClientPtr();
        setSubRcfClientPtr( RcfClientPtr() );
        if (subRcfClientPtr)
        {
            subRcfClientPtr->getClientStub().disconnect();
            subRcfClientPtr.reset();
        }

        if (mTransport.get())
        {
            RCF_LOG_2()(this)(endpoint)
                << "RcfClient - disconnecting from server.";

            mTransport->disconnect(mConnectTimeoutMs);
            mConnected = false;
        }

        if (mBatchBufferPtr)
        {
            mBatchBufferPtr->resize(0);
        }

        mAsyncCallback = boost::function0<void>();
    }

    bool ClientStub::isConnected()
    {
        return mTransport.get() && mTransport->isConnected();
    }

    void ClientStub::setConnected(bool connected)
    {
        mConnected = connected;
    }

    void ClientStub::setMessageFilters()
    {
        setMessageFilters( std::vector<FilterPtr>());
    }

    void ClientStub::setMessageFilters(const std::vector<FilterPtr> &filters)
    {
        mMessageFilters.assign(filters.begin(), filters.end());
        RCF::connectFilters(mMessageFilters);
    }

    void ClientStub::setMessageFilters(FilterPtr filterPtr)
    {
        std::vector<FilterPtr> filters;
        filters.push_back(filterPtr);
        setMessageFilters(filters);
    }

    const std::vector<FilterPtr> &ClientStub::getMessageFilters()
    {
        return mMessageFilters;
    }

    bool ClientStub::isClientStub() const 
    { 
        return true;
    }

    void ClientStub::setRemoteCallTimeoutMs(unsigned int remoteCallTimeoutMs)
    {
        mRemoteCallTimeoutMs = remoteCallTimeoutMs;
    }

    unsigned int ClientStub::getRemoteCallTimeoutMs() const
    {
        return mRemoteCallTimeoutMs;
    }

    void ClientStub::setConnectTimeoutMs(unsigned int connectTimeoutMs)
    {
        mConnectTimeoutMs = connectTimeoutMs;
    }

    unsigned int ClientStub::getConnectTimeoutMs() const
    {
        return mConnectTimeoutMs;
    }

    void ClientStub::setAutoVersioning(bool autoVersioning)
    {
        mAutoVersioning = autoVersioning;
    }

    bool ClientStub::getAutoVersioning() const
    {
        return mAutoVersioning;
    }

    void ClientStub::setRuntimeVersion(boost::uint32_t version)
    {
        mRuntimeVersion = version;
    }

    boost::uint32_t ClientStub::getRuntimeVersion() const
    {
        return mRuntimeVersion;
    }

    void ClientStub::setArchiveVersion(boost::uint32_t version)
    {
        mArchiveVersion = version;
    }

    boost::uint32_t ClientStub::getArchiveVersion() const
    {
        return mArchiveVersion;
    }

    void ClientStub::setTries(std::size_t tries)
    {
        mTries = tries;
    }

    std::size_t ClientStub::getTries() const
    {
        return mTries;
    }

    CurrentClientStubSentry::CurrentClientStubSentry(ClientStub & clientStub)
    {
        pushTlsClientStub(&clientStub);
    }

    CurrentClientStubSentry::~CurrentClientStubSentry()
    {
        popTlsClientStub();
    }

    void ClientStub::onError(const std::exception &e)
    {
        if (mAsync)
        {
            scheduleAmiNotification();
        }

        mAsyncOpType = None;

        const RemoteException *pRcfRE = 
            dynamic_cast<const RemoteException *>(&e);

        const Exception *pRcfE = 
            dynamic_cast<const Exception *>(&e);

        if (pRcfRE)
        {
            mEncodedByteBuffers.resize(0);
            if (shouldDisconnectOnRemoteError( pRcfRE->getError() ))
            {
                disconnect();
            }

            mAsyncException.reset( pRcfRE->clone().release() );
        }
        else if (pRcfE)
        {
            mEncodedByteBuffers.resize(0);
            disconnect();
            mAsyncException.reset( pRcfE->clone().release() );
        }
        else
        {
            mEncodedByteBuffers.resize(0);
            disconnect();

            mAsyncException.reset(new Exception(e.what()));
        }
    }

    void ClientStub::onTimerExpired()
    {
        AsyncOpType opType = mAsyncOpType;
        mAsyncOpType = None;

        if (opType == Wait)
        {
            scheduleAmiNotification();
        }
        else
        {
            switch(opType)
            {
            case Connect:
                RCF_ASSERT(mEndpoint.get());
                
                onError(RCF::Exception(_RcfError_ClientConnectTimeout(
                    mConnectTimeoutMs, 
                    mEndpoint->asString())));

                break;

            case Write:
                onError(RCF::Exception(_RcfError_ClientWriteTimeout()));
                break;

            case Read: 
                onError(RCF::Exception(_RcfError_ClientReadTimeout()));
                break;

            default:
                RCF_ASSERT(0)(opType);
            };
        }        
    }

    void ClientStub::setUserData(boost::any userData)
    {
        mUserData = userData;
    }

    boost::any ClientStub::getUserData()
    {
        return mUserData;
    }

    void ClientStub::setInterfaceName(const std::string & interfaceName)
    {
        mInterfaceName = interfaceName;
    }

    const std::string & ClientStub::getInterfaceName()
    {
        return mInterfaceName;
    }

    SerializationProtocolIn & ClientStub::getSpIn()
    {
        return mIn;
    }

    SerializationProtocolOut & ClientStub::getSpOut()
    {
        return mOut;
    }

    void ClientStub::setPingBackIntervalMs(int pingBackIntervalMs)
    {
        mPingBackIntervalMs = pingBackIntervalMs;
    }
    
    int ClientStub::getPingBackIntervalMs()
    {
        return mPingBackIntervalMs;
    }

    std::size_t ClientStub::getPingBackCount()
    {
        return mPingBackCount;
    }

    boost::uint32_t ClientStub::getPingBackTimeStamp()
    {
        return mPingBackTimeStamp;
    }

    FutureImpl<Void> ClientStub::ping()
    {
        return ping( CallOptions() );
    }

    FutureImpl<Void> ClientStub::ping(const CallOptions & callOptions)
    {
        typedef Void V;

        CurrentClientStubSentry sentry(*this);

        setAsync(false);

        return RCF::FutureImpl<V>(
            ::RCF::AllocateClientParameters<
                V
                    ,
                V,V,V,V,V,V,V,V,V,V,V,V,V,V,V >()(
                    *this
                        ,
                    V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V(),V()).r.get(),
            *this,
            getInterfaceName(),
            -1,
            callOptions.apply(*this),
            "ping",
            "V0");

    }   

    // Take the proposed timeout and cut it down to accommodate client progress 
    // callbacks and checking of ping back interval.

    boost::uint32_t ClientStub::generatePollingTimeout(boost::uint32_t timeoutMs)
    {
        boost::uint32_t timeNowMs = RCF::getCurrentTimeMs();

        boost::uint32_t timeToNextTimerCallbackMs = mNextTimerCallbackMs ?
            mNextTimerCallbackMs - timeNowMs:
            boost::uint32_t(-1);

        boost::uint32_t timeToNextPingBackCheckMs = mNextPingBackCheckMs ?
            mNextPingBackCheckMs - timeNowMs:
            boost::uint32_t(-1);

        return 
            RCF_MIN( 
                RCF_MIN(timeToNextTimerCallbackMs, timeToNextPingBackCheckMs), 
                timeoutMs);
    }

    void ClientStub::onPollingTimeout()
    {
        // Check whether we need to fire a client progress timer callback.
        if (mNextTimerCallbackMs && 0 == generateTimeoutMs(mNextTimerCallbackMs))
        {
            ClientProgress::Action action = ClientProgress::Continue;

            mClientProgressPtr->mProgressCallback(
                0,
                0,
                ClientProgress::Timer,
                ClientProgress::Receive,
                action);

            RCF_VERIFY(
                action == ClientProgress::Continue,
                Exception(_RcfError_ClientCancel()))
                (mTimerIntervalMs);

            mNextTimerCallbackMs = 
                RCF::getCurrentTimeMs() + mTimerIntervalMs;

            mNextTimerCallbackMs |= 1;
        }

        // Check that pingbacks have been received.
        if (mNextPingBackCheckMs && 0 == generateTimeoutMs(mNextPingBackCheckMs))
        {
            boost::uint32_t timeNowMs = RCF::getCurrentTimeMs();

            boost::uint32_t timeSinceLastPingBackMs = 
                timeNowMs - mPingBackTimeStamp;

            // Checking for subsequent pingbacks.
            RCF_VERIFY(
                timeSinceLastPingBackMs < mPingBackCheckIntervalMs,
                Exception(_RcfError_PingBackTimeout(mPingBackCheckIntervalMs)))
                (mPingBackCheckIntervalMs);

            // Setup polling for next pingback.
            mPingBackCheckIntervalMs = 3 * mPingBackIntervalMs;

            mNextPingBackCheckMs = 
                RCF::getCurrentTimeMs() + mPingBackCheckIntervalMs;

            mNextPingBackCheckMs |= 1;
        }

    }

    void ClientStub::onUiMessage()
    {
        ClientProgress::Action action = ClientProgress::Continue;

        mClientProgressPtr->mProgressCallback(
            0,
            0,
            ClientProgress::UiMessage,
            ClientProgress::Receive,
            action);

        RCF_VERIFY(
            action != ClientProgress::Cancel,
            Exception(_RcfError_ClientCancel()))
            (mClientProgressPtr->mUiMessageFilter);

        // a sample message filter

        //MSG msg = {0};
        //while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        //{
        //    if (msg.message == WM_QUIT)
        //    {
        //
        //    }
        //    else if (msg.message == WM_PAINT)
        //    {
        //        TranslateMessage(&msg);
        //        DispatchMessage(&msg);
        //    }
        //}

    }

    //**************************************************************************
    // Synchronous transport filter requests.

    ByteBuffer ClientStub::getOutOfBandRequest()
    {
        return mRequest.mOutOfBandRequest;
    }

    void ClientStub::setOutofBandRequest(ByteBuffer requestBuffer)
    {
        mRequest.mOutOfBandRequest = requestBuffer;
    }

    ByteBuffer ClientStub::getOutOfBandResponse()
    {
        return mRequest.mOutOfBandResponse;
    }

    void ClientStub::setOutofBandResponse(ByteBuffer responseBuffer)
    {
        mRequest.mOutOfBandResponse = responseBuffer;
    }

    void ClientStub::requestTransportFilters(const std::vector<FilterPtr> &filters)
    {
        if (getRuntimeVersion() <= 11)
        {
            requestTransportFilters_Legacy(filters);
            return;
        }

        ClientStub stub(*this);
        stub.setTransport( releaseTransport());
        stub.setTargetToken( Token());

        RestoreClientTransportGuard guard(*this, stub);

        // Set OOB request.
        OobRequestTransportFilters msg(getRuntimeVersion(), filters);
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

        for (std::size_t i=0; i<filters.size(); ++i)
        {
            filters[i]->resetState();
        }

        stub.getTransport().setTransportFilters(filters);
    }

    void ClientStub::requestTransportFilters(FilterPtr filterPtr)
    {
        std::vector<FilterPtr> filters;
        if (filterPtr.get())
        {
            filters.push_back(filterPtr);
        }
        requestTransportFilters(filters);
    }

    void ClientStub::requestTransportFilters()
    {
        requestTransportFilters( std::vector<FilterPtr>());
    }

    void ClientStub::clearTransportFilters()
    {
        disconnect();
        if (mTransport.get())
        {
            mTransport->setTransportFilters( std::vector<FilterPtr>());
        }
    }

    //**************************************************************************
    // Asynchronous transport filter requests.

    void onRtfCompleted(
        RCF::Future<Void>                           fv, 
        RcfClientPtr                                rtfClientPtr,
        ClientStub &                                clientStubOrig,
        boost::shared_ptr<std::vector<FilterPtr> >  filters, 
        boost::function0<void>                      onCompletion)
    {
        ClientStubPtr rtfStubPtr = rtfClientPtr->getClientStubPtr();
        clientStubOrig.setTransport( rtfStubPtr->releaseTransport() );
        clientStubOrig.setSubRcfClientPtr( RcfClientPtr() );

        std::auto_ptr<Exception> ePtr = fv.getAsyncException();
        if ( ePtr.get() )
        {
            clientStubOrig.setAsyncException(ePtr);
        }
        else
        {
            // Get OOB response.
            OobRequestTransportFilters msg(clientStubOrig.getRuntimeVersion());
            ByteBuffer controlResponse = rtfStubPtr->getOutOfBandResponse();
            rtfStubPtr->setOutofBandRequest(ByteBuffer());
            rtfStubPtr->setOutofBandResponse(ByteBuffer());
            msg.decodeResponse(controlResponse);

            int ret = msg.mResponseError; 
            if (ret != RcfError_Ok)
            {
                ePtr.reset( new RemoteException(Error(ret)) );
                clientStubOrig.setAsyncException(ePtr);
            }
            else
            {
                for (std::size_t i=0; i<filters->size(); ++i)
                {
                    (*filters)[i]->resetState();
                }
                clientStubOrig.getTransport().setTransportFilters(*filters);
            }
        }
        onCompletion();
    }


    void ClientStub::requestTransportFiltersAsync(
        const std::vector<FilterPtr> &filters,
        boost::function0<void> onCompletion)
    {       
        if (getRuntimeVersion() <= 11)
        {
            requestTransportFiltersAsync_Legacy(filters, onCompletion);
            return;
        }

        RcfClientPtr rtfClientPtr( new I_RcfClient("", *this) );
        rtfClientPtr->getClientStub().setTransport( releaseTransport());
        rtfClientPtr->getClientStub().setTargetToken( Token());
        setSubRcfClientPtr(rtfClientPtr);

        setAsync(true);

        OobRequestTransportFilters msg(getRuntimeVersion(), filters);
        ByteBuffer controlRequest;
        msg.encodeRequest(controlRequest);
        rtfClientPtr->getClientStub().setOutofBandRequest(controlRequest);

        Future<Void> fv;

        boost::shared_ptr<std::vector<FilterPtr> > filtersPtr(
            new std::vector<FilterPtr>(filters) );

        fv = rtfClientPtr->getClientStub().ping( RCF::AsyncTwoway( boost::bind(
            &onRtfCompleted, 
            fv,
            rtfClientPtr,
            boost::ref(*this),
            filtersPtr,
            onCompletion)));
    }

    void ClientStub::requestTransportFiltersAsync(
        FilterPtr filterPtr,
        boost::function0<void> onCompletion)
    {
        std::vector<FilterPtr> filters;
        if (filterPtr.get())
        {
            filters.push_back(filterPtr);
        }
        requestTransportFiltersAsync(filters, onCompletion);
    }

    // Batching

    void ClientStub::enableBatching()
    {
        setRemoteCallSemantics(Oneway);

        mBatchMode = true;
        if (!mBatchBufferPtr)
        {
            mBatchBufferPtr.reset( new ReallocBuffer() );
        }
        mBatchBufferPtr->resize(0);
        mBatchCount = 0;
        mBatchMessageCount = 0;
    }

    void ClientStub::disableBatching(bool flush)
    {
        if (flush)
        {
            flushBatch();
        }
        mBatchMode = false;
        mBatchBufferPtr->resize(0);
        mBatchMessageCount = 0;
    }

    void ClientStub::flushBatch(unsigned int timeoutMs)
    {
        CurrentClientStubSentry sentry(*this);

        if (timeoutMs == 0)
        {
            timeoutMs = getRemoteCallTimeoutMs();
        }

        try
        {
            std::vector<ByteBuffer> buffers;
            buffers.push_back( ByteBuffer(mBatchBufferPtr) );
            int err = getTransport().send(*this, buffers, timeoutMs);
            RCF_UNUSED_VARIABLE(err);

            mBatchBufferPtr->resize(0);

            ++mBatchCount;
            mBatchMessageCount = 0;
        }
        catch(const RemoteException & e)
        {
            RCF_UNUSED_VARIABLE(e);
            mEncodedByteBuffers.resize(0);
            throw;
        }
        catch(...)
        {
            mEncodedByteBuffers.resize(0);
            disconnect();
            throw;
        }
    }

    void ClientStub::setMaxBatchMessageLength(boost::uint32_t maxBatchMessageLength)
    {
        mBatchMaxMessageLength = maxBatchMessageLength;
    }

    boost::uint32_t ClientStub::getMaxBatchMessageLength()
    {
        return mBatchMaxMessageLength;
    }

    boost::uint32_t ClientStub::getBatchesSent()
    {
        return mBatchCount;
    }

    boost::uint32_t ClientStub::getMessagesInCurrentBatch()
    {
        return mBatchMessageCount;
    }

    void ClientStub::setRequestUserData(const std::string & userData)
    {
        mRequest.mRequestUserData = ByteBuffer(userData);
    }

    std::string ClientStub::getRequestUserData()
    {
        if ( mRequest.mRequestUserData.isEmpty() )
        {
            return std::string();
        }

        return std::string(
            mRequest.mRequestUserData.getPtr(), 
            mRequest.mRequestUserData.getLength());
    }

    void ClientStub::setResponseUserData(const std::string & userData)
    {
        mRequest.mResponseUserData = ByteBuffer(userData);
    }

    std::string ClientStub::getResponseUserData()
    {
        if ( mRequest.mResponseUserData.isEmpty() )
        {
            return std::string();
        }

        return std::string(
            mRequest.mResponseUserData.getPtr(), 
            mRequest.mResponseUserData.getLength());
    }

    void ClientStub::setHttpProxy(const std::string & httpProxy)
    {
        if (httpProxy != mHttpProxy)
        {
            mHttpProxy = httpProxy;
            disconnect();
        }
    }

    std::string ClientStub::getHttpProxy()
    {
        return mHttpProxy;
    }

    void ClientStub::setHttpProxyPort(int httpProxyPort)
    {
        if (httpProxyPort != mHttpProxyPort)
        {
            mHttpProxyPort = httpProxyPort;
            disconnect();
        }
    }

    int ClientStub::getHttpProxyPort()
    {
        return mHttpProxyPort;
    }

    void ClientStub::setTransportProtocol(TransportProtocol protocol)
    {
        if (mTransportProtocol != protocol)
        {
            mTransportProtocol = protocol;
            if ( mTransport.get() )
            {
                RcfSessionWeakPtr sessionWeakPtr = getTransport().getRcfSession();
                bool isRegularConnection = (sessionWeakPtr == RcfSessionWeakPtr());
                if ( isRegularConnection )
                {
                    disconnect();
                    clearTransportFilters();
                }
                else
                {
                    mSetTransportProtocol = true;
                }
            }
        }
    }

    TransportProtocol ClientStub::getTransportProtocol()
    {
        return mTransportProtocol;
    }

    TransportType ClientStub::getTransportType()
    {
        instantiateTransport();
        return mTransport->getTransportType();
    }

    void ClientStub::setUsername(const tstring & username)
    {
        mUsername = username;
    }

    tstring ClientStub::getUsername()
    {
        return mUsername;
    }

    void ClientStub::setPassword(const tstring & password)
    {
        mPassword = password;
    }

    tstring ClientStub::getPassword()
    {
        return mPassword;
    }

    void ClientStub::setKerberosSpn(const tstring & kerberosSpn)
    {
        mKerberosSpn = kerberosSpn;
    }

    tstring ClientStub::getKerberosSpn()
    {
        return mKerberosSpn;
    }

    void ClientStub::setEnableCompression(bool enableCompression)
    {
        if (mEnableCompression != enableCompression)
        {
            mEnableCompression = enableCompression;
            disconnect();
            clearTransportFilters();
        }
    }

    bool ClientStub::getEnableCompression()
    {
        return mEnableCompression;
    }

    void ClientStub::setCertificate(CertificatePtr certificatePtr)
    {
        mCertificatePtr = certificatePtr;
    }

    CertificatePtr ClientStub::getCertificate()
    {
        return mCertificatePtr;
    }

    void ClientStub::setCaCertificate(CertificatePtr caCertificatePtr)
    {
        mCaCertificatePtr = caCertificatePtr;

        mCertificateValidationCb.clear();
        mSchannelCertificateValidation.clear();
    }

    CertificatePtr ClientStub::getCaCertificate()
    {
        return mCaCertificatePtr;
    }

    void ClientStub::setCertificateValidationCallback(
        CertificateValidationCb certificateValidationCb)
    {
        mCertificateValidationCb = certificateValidationCb;

        mCaCertificatePtr.reset();
        mSchannelCertificateValidation.clear();
    }

    const ClientStub::CertificateValidationCb & ClientStub::getCertificateValidationCallback() const
    {
        return mCertificateValidationCb;
    }

    void ClientStub::setEnableSchannelCertificateValidation(const tstring & peerName)
    {
        mSchannelCertificateValidation = peerName;

        mCaCertificatePtr.reset();
        mCertificateValidationCb.clear();
    }

    tstring ClientStub::getEnableSchannelCertificateValidation() const
    {
        return mSchannelCertificateValidation;
    }

    void ClientStub::setOpenSslCipherSuite(const std::string & cipherSuite)
    {
        mOpenSslCipherSuite = cipherSuite;
    }

    std::string ClientStub::getOpenSslCipherSuite() const
    {
        return mOpenSslCipherSuite;
    }

    void ClientStub::setSslImplementation(SslImplementation sslImplementation)
    {
        mSslImplementation = sslImplementation;
    }

    SslImplementation ClientStub::getSslImplementation() const
    {
        return mSslImplementation;
    }

#ifdef BOOST_WINDOWS

    void ClientStub::setWindowsImpersonationToken(HANDLE hToken)
    {
        mWindowsImpersonationToken.reset( new HANDLE(hToken) );
    }

    HANDLE ClientStub::getWindowsImpersonationToken() const
    {
        if (mWindowsImpersonationToken.get())
        {
            return *mWindowsImpersonationToken;
        }
        return INVALID_HANDLE_VALUE;
    }

#endif

    RestoreClientTransportGuard::RestoreClientTransportGuard(
        ClientStub &client, 
        ClientStub &clientTemp) :
            mClient(client),
            mClientTemp(clientTemp)
    {}

    RestoreClientTransportGuard::~RestoreClientTransportGuard()
    {
        RCF_DTOR_BEGIN
            mClient.setTransport(mClientTemp.releaseTransport());
        RCF_DTOR_END
    }

} // namespace RCF
