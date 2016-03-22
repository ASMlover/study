
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

#ifndef INCLUDE_RCF_CLIENTSTUB_HPP
#define INCLUDE_RCF_CLIENTSTUB_HPP

#include <string>
#include <vector>
#include <memory>

#include <boost/scoped_ptr.hpp>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <RCF/Certificate.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/Filter.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Endpoint.hpp>
#include <RCF/Enums.hpp>
#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/MethodInvocation.hpp>
#include <RCF/SerializationProtocol_Base.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/Token.hpp>

#if RCF_FEATURE_FILETRANSFER==1
#include <RCF/FileStream.hpp>
#include <RCF/FileDownload.hpp>
#include <RCF/FileUpload.hpp>
#endif

namespace RCF {

    template<typename T>
    class FutureImpl;

    struct Void;

    class CallOptions;

    class ConnectionResetGuard;

    class I_Parameters;

    /// Sets the default SSL implementation to use (OpenSSL or Schannel). The default is OpenSSL.
    RCF_EXPORT void                 setDefaultSslImplementation(SslImplementation sslImplementation);

    /// Gets the default SSL implementation.
    RCF_EXPORT SslImplementation    getDefaultSslImplementation();

    RCF_EXPORT void         setDefaultConnectTimeoutMs(unsigned int connectTimeoutMs);
    RCF_EXPORT unsigned int getDefaultConnectTimeoutMs();

    RCF_EXPORT void         setDefaultRemoteCallTimeoutMs(unsigned int remoteCallTimeoutMs);
    RCF_EXPORT unsigned int getDefaultRemoteCallTimeoutMs();

    RCF_EXPORT void         setDefaultNativeWstringSerialization(bool enable);
    RCF_EXPORT bool         getDefaultNativeWstringSerialization();

    class ClientStub;

    typedef boost::shared_ptr<ClientStub> ClientStubPtr;

    typedef Token FileTransferToken;

    class ClientProgress;
    typedef boost::shared_ptr<ClientProgress> ClientProgressPtr;

    class ClientTransport;
    typedef std::auto_ptr<ClientTransport> ClientTransportAutoPtr;

    class I_RcfClient;
    typedef boost::shared_ptr<I_RcfClient> RcfClientPtr;

    class I_Future;
    class IpClientTransport;

    template<typename T>
    class FutureImpl;

    class FileTransferProgress;

    class OpenSslEncryptionFilter;

    class SspiFilter;

    typedef boost::function1<void, const FileTransferProgress &> FileProgressCb;

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8,
        typename A9,
        typename A10,
        typename A11,
        typename A12,
        typename A13,
        typename A14,
        typename A15>
    class AllocateClientParameters;

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8,
        typename A9,
        typename A10,
        typename A11,
        typename A12,
        typename A13,
        typename A14,
        typename A15>
    class ClientParameters;

    class OverlappedAmi;
    typedef boost::shared_ptr<OverlappedAmi> OverlappedAmiPtr;

    class RCF_EXPORT CurrentClientStubSentry
    {
    public:
        CurrentClientStubSentry(ClientStub & clientStub);
        ~CurrentClientStubSentry();
    };

    class RCF_EXPORT ClientStub : 
        public ClientTransportCallback, 
        public boost::enable_shared_from_this<ClientStub>
    {
    public:
        ClientStub(const std::string & interfaceName);
        ClientStub(const std::string & interfaceName, const std::string &objectName);
        ClientStub(const ClientStub & rhs);
        ~ClientStub();

        ClientStub &operator=(const ClientStub & rhs);

    private:
        void                    init(const std::string & interfaceName, const std::string & objectName);
        void                    assign(const ClientStub & rhs);

    public:

        void                    setEndpoint(const Endpoint &endpoint);
        void                    setEndpoint(EndpointPtr endpointPtr);
        EndpointPtr             getEndpoint() const;
        Token                   getTargetToken() const;
        void                    setTargetToken(Token token);
        const std::string &     getTargetName() const;
        void                    setTargetName(const std::string &targetName);
        void                    setInterfaceName(const std::string & interfaceName);
        const std::string &     getInterfaceName();
        RemoteCallSemantics     getRemoteCallSemantics() const;
        void                    setRemoteCallSemantics(RemoteCallSemantics defaultCallingSemantics);

        // Deprecated - use getRemoteCallSemantics()/setRemoteCallSemantics instead.
        RemoteCallSemantics     getDefaultCallingSemantics() const;
        void                    setDefaultCallingSemantics(RemoteCallSemantics defaultCallingSemantics);

        void                    setSerializationProtocol(SerializationProtocol protocol);
        SerializationProtocol   getSerializationProtocol() const;

        void                    setMarshalingProtocol(MarshalingProtocol protocol);
        MarshalingProtocol      getMarshalingProtocol() const;

        bool                    getNativeWstringSerialization();
        void                    setNativeWstringSerialization(bool enable);

        void                    setEnableSfPointerTracking(bool enable);
        bool                    getEnableSfPointerTracking() const;

        void                    setTransport(ClientTransportAutoPtr transport);

        ClientTransport&        getTransport();
        IpClientTransport &     getIpTransport();

        ClientTransportAutoPtr  releaseTransport();

        void                    instantiateTransport();
        void                    connect();
        void                    connectAsync(boost::function0<void> onCompletion);
        void                    wait(boost::function0<void> onCompletion, boost::uint32_t timeoutMs);
        void                    disconnect();
        bool                    isConnected();
        void                    setConnected(bool connected);

        void                    setMessageFilters(const std::vector<FilterPtr> &filters);
        void                    setMessageFilters();
        void                    setMessageFilters(FilterPtr filterPtr);

        const std::vector<FilterPtr> &
                                getMessageFilters();

        virtual bool            isClientStub() const;


        // Synchronous transport filter requests.
        void                    requestTransportFilters_Legacy(const std::vector<FilterPtr> &filters);
        void                    requestTransportFilters(const std::vector<FilterPtr> &filters);
        void                    requestTransportFilters(FilterPtr filterPtr);
        void                    requestTransportFilters();

        void                    clearTransportFilters();

        // Asynchronous transport filter requests.
        void                    requestTransportFiltersAsync_Legacy(
                                    const std::vector<FilterPtr> &filters,
                                    boost::function0<void> onCompletion);

        void                    requestTransportFiltersAsync(
                                    const std::vector<FilterPtr> &filters,
                                    boost::function0<void> onCompletion);

        void                    requestTransportFiltersAsync(
                                    FilterPtr filterPtr,
                                    boost::function0<void> onCompletion);


        void                    setRemoteCallTimeoutMs(unsigned int remoteCallTimeoutMs);
        unsigned int            getRemoteCallTimeoutMs() const;

        void                    setConnectTimeoutMs(unsigned int connectTimeoutMs);
        unsigned int            getConnectTimeoutMs() const;

        void                    setAutoReconnect(bool autoReconnect);
        bool                    getAutoReconnect() const;

        void                    setAutoVersioning(bool autoVersioning);
        bool                    getAutoVersioning() const;

        void                    setRuntimeVersion(boost::uint32_t version);
        boost::uint32_t         getRuntimeVersion() const;

        void                    setArchiveVersion(boost::uint32_t version);
        boost::uint32_t         getArchiveVersion() const;

        void                    setClientProgressPtr(ClientProgressPtr clientProgressPtr);

        ClientProgressPtr       getClientProgressPtr() const;

        typedef     ClientProgress::ProgressCallback RemoteCallProgressCallback;

        void                    setRemoteCallProgressCallback(
                                    RemoteCallProgressCallback cb, 
                                    boost::uint32_t callbackIntervalMs);

        void                    setTries(std::size_t tries);
        std::size_t             getTries() const;

        void                    setUserData(boost::any userData);
        boost::any              getUserData();

        //**********************************************************************
        // These functions involve network calls.

        // Synchronous versions.

#if RCF_FEATURE_LEGACY==1

        void                    createRemoteObject(const std::string &objectName = "");
        void                    deleteRemoteObject();

        void                    createRemoteSessionObject(const std::string &objectName = "");
        void                    deleteRemoteSessionObject();

#endif

#if RCF_FEATURE_FILETRANSFER==1
        void                    setFileProgressCallback(FileProgressCb fileProgressCb);
        void                    setFileProgressCallback() { setFileProgressCallback( FileProgressCb() ); }

        void                    uploadFiles(
                                    const std::string & whichFile, 
                                    std::string & uploadId,
                                    boost::uint32_t chunkSize,
                                    boost::uint32_t transferRateBps,
                                    boost::uint32_t sessionLocalId);

        void                    uploadFiles(
                                    const FileManifest & whichFile, 
                                    std::string & uploadId,
                                    boost::uint32_t chunkSize,
                                    boost::uint32_t transferRateBps,
                                    boost::uint32_t sessionLocalId);

        void                    downloadFiles(
                                    const std::string & downloadLocation, 
                                    FileManifest & manifest,
                                    boost::uint32_t chunkSize, 
                                    boost::uint32_t transferRateBps,
                                    boost::uint32_t sessionLocalId);

        boost::uint32_t         addUploadStream(FileUpload fileStream);
        void                    processUploadStreams();

        boost::uint32_t         addDownloadStream(FileDownload fileStream);

        // For testing.
        void                    setTransferWindowS(boost::uint32_t transferWindowS);
        boost::uint32_t         getTransferWindowS();
#endif

        FutureImpl<Void>        ping();
        FutureImpl<Void>        ping(const CallOptions & callOptions);

        ByteBuffer              getOutOfBandRequest();
        void                    setOutofBandRequest(ByteBuffer requestBuffer);

        ByteBuffer              getOutOfBandResponse();
        void                    setOutofBandResponse(ByteBuffer responseBuffer);

        FutureImpl<Void>        doControlMessage(
                                    const CallOptions &     callOptions, 
                                    ByteBuffer              controlRequest);

        //**********************************************************************

        void                    setPingBackIntervalMs(int pingBackIntervalMs);
        int                     getPingBackIntervalMs();

        std::size_t             getPingBackCount();
        boost::uint32_t         getPingBackTimeStamp();
        
        void                    clearParameters();

        SerializationProtocolIn &   getSpIn();
        SerializationProtocolOut &  getSpOut();

        std::auto_ptr<Exception>    getAsyncException();
        void                        setAsyncException(std::auto_ptr<Exception>);
        bool                        hasAsyncException();

        boost::uint32_t         generatePollingTimeout(boost::uint32_t timeoutMs);
        void                    onPollingTimeout();
        void                    onUiMessage();

        friend class CallOptions;

    private:

        friend class FutureImplBase;

        template<
            typename R, 
            typename A1,
            typename A2,
            typename A3,
            typename A4,
            typename A5,
            typename A6,
            typename A7,
            typename A8,
            typename A9,
            typename A10,
            typename A11,
            typename A12,
            typename A13,
            typename A14,
            typename A15>
        friend class AllocateClientParameters;

        template<
            typename R, 
            typename A1,
            typename A2,
            typename A3,
            typename A4,
            typename A5,
            typename A6,
            typename A7,
            typename A8,
            typename A9,
            typename A10,
            typename A11,
            typename A12,
            typename A13,
            typename A14,
            typename A15>
        friend class ClientParameters;

        Token                       mToken;
        RemoteCallSemantics         mDefaultCallingSemantics;
        SerializationProtocol       mProtocol;
        MarshalingProtocol          mMarshalingProtocol; 
        std::string                 mEndpointName;
        std::string                 mObjectName;
        std::string                 mInterfaceName;

        unsigned int                mRemoteCallTimeoutMs;
        unsigned int                mConnectTimeoutMs;
        bool                        mAutoReconnect;
        bool                        mConnected;
        std::size_t                 mTries;

        EndpointPtr                 mEndpoint;
        ClientTransportAutoPtr      mTransport;

        VectorFilter                mMessageFilters;

        ClientProgressPtr           mClientProgressPtr;

        bool                        mAutoVersioning;
        boost::uint32_t             mRuntimeVersion;
        boost::uint32_t             mArchiveVersion;

        bool                        mUseNativeWstringSerialization;
        bool                        mEnableSfPointerTracking;

        std::vector<I_Future *>     mFutures;
        boost::any                  mUserData;        

        MethodInvocationRequest     mRequest;
        SerializationProtocolIn     mIn;
        SerializationProtocolOut    mOut;

        bool                        mAsync;
        AsyncOpType                 mAsyncOpType;
        boost::function0<void>      mAsyncCallback;
        std::auto_ptr<Exception>    mAsyncException;
        unsigned int                mEndTimeMs;
        bool                        mRetry;
        RemoteCallSemantics         mRcs;
        ByteBuffer                  mEncodedByteBuffer;
        std::vector<ByteBuffer>     mEncodedByteBuffers;

        std::vector<char>           mRetValVec;
        std::vector<char>           mParametersVec;
        I_Parameters *              mpParameters;

        boost::uint32_t             mPingBackIntervalMs;
        boost::uint32_t             mPingBackTimeStamp;
        std::size_t                 mPingBackCount;

        boost::uint32_t             mNextTimerCallbackMs;
        boost::uint32_t             mNextPingBackCheckMs;
        boost::uint32_t             mPingBackCheckIntervalMs;
        boost::uint32_t             mTimerIntervalMs;

        MutexPtr                    mSignalledMutexPtr;
        ConditionPtr                mSignalledConditionPtr;
        LockPtr                     mSignalledLockPtr;

        bool                        mSignalled;

        Mutex                       mSubRcfClientMutex;
        RcfClientPtr                mSubRcfClientPtr;

        bool                        mBatchMode;
        ReallocBufferPtr            mBatchBufferPtr;
        ReallocBuffer               mBatchBufferTemp;
        boost::uint32_t             mBatchMaxMessageLength;
        boost::uint32_t             mBatchCount;
        boost::uint32_t             mBatchMessageCount;

        // Set to true if we need to set the transport protocol without disconnecting.
        bool                        mSetTransportProtocol;  

        void                enrol(
                                I_Future *pFuture);

        void                init( 
                                const std::string & interfaceName,
                                int fnId, 
                                RCF::RemoteCallSemantics rcs);

        void                beginCall();

        void                beginSend();

        void                beginReceive();

        void                call( 
                                RCF::RemoteCallSemantics rcs);

        void                onConnectCompleted(
                                bool alreadyConnected = false);

        void                onRequestTransportFiltersCompleted();

        void                doBatching();

        void                onSendCompleted();

        void                onReceiveCompleted();

        void                onTimerExpired();

        void                onError(
                                const std::exception &e);
   
        void                setAsyncCallback(
                                boost::function0<void> callback);        

        void                onException(
                                const Exception & e);

        void                scheduleAmiNotification();

        void                createFilterSequence(std::vector<FilterPtr> & filters);


    public:

        std::vector<char> & getRetValVec() { return mRetValVec; }

        // Batching

        void                enableBatching();
        void                disableBatching(bool flush = true);
        void                flushBatch(unsigned int timeoutMs = 0);

        void                setMaxBatchMessageLength(boost::uint32_t maxBatchMessageLength);
        boost::uint32_t     getMaxBatchMessageLength();

        boost::uint32_t     getBatchesSent();
        boost::uint32_t     getMessagesInCurrentBatch();

        // Async

        void                setAsync(bool async);
        bool                getAsync();

        bool                ready();
        void                waitForReady(boost::uint32_t timeoutMs = 0);
        void                cancel();

        void                setSubRcfClientPtr(RcfClientPtr clientStubPtr);
        RcfClientPtr        getSubRcfClientPtr();

        // User data

        void                setRequestUserData(const std::string & userData);
        std::string         getRequestUserData();

        void                setResponseUserData(const std::string & userData);
        std::string         getResponseUserData();

#if RCF_FEATURE_FILETRANSFER==1
        FileProgressCb              mFileProgressCb;

        std::vector<FileUpload>     mUploadStreams;
        std::vector<FileDownload>   mDownloadStreams;       
#endif

        boost::uint32_t             mTransferWindowS;    

        RecursionState<int, int>    mRecursionState;

        std::string                 mCurrentCallDesc;
        bool                        mCallInProgress;

    public:

        void                setHttpProxy(const std::string & httpProxy);
        std::string         getHttpProxy();

        void                setHttpProxyPort(int httpProxyPort);
        int                 getHttpProxyPort();
        
        void                setTransportProtocol(TransportProtocol protocol);
        TransportProtocol   getTransportProtocol();

        TransportType       getTransportType();

        void                setUsername(const tstring & username);
        tstring             getUsername();

        void                setPassword(const tstring & password);
        tstring             getPassword();

        void                setKerberosSpn(const tstring & kerberosSpn);
        tstring             getKerberosSpn();

        void                setEnableCompression(bool enableCompression);
        bool                getEnableCompression();

        void                setCertificate(CertificatePtr certificatePtr);
        CertificatePtr      getCertificate();

        void                setCaCertificate(CertificatePtr certificatePtr);
        CertificatePtr      getCaCertificate();

        void                setOpenSslCipherSuite(const std::string & cipherSuite);
        std::string         getOpenSslCipherSuite() const;

        void                setEnableSchannelCertificateValidation(const tstring & peerName);
        tstring             getEnableSchannelCertificateValidation() const;

        typedef boost::function<bool(Certificate *)> CertificateValidationCb;
        void                                setCertificateValidationCallback(CertificateValidationCb certificateValidationCb);
        const CertificateValidationCb &     getCertificateValidationCallback() const;

        void                setSslImplementation(SslImplementation sslImplementation);
        SslImplementation   getSslImplementation() const;

#ifdef BOOST_WINDOWS
        void                setWindowsImpersonationToken(HANDLE hToken);
        HANDLE              getWindowsImpersonationToken() const;

    private:
        std::auto_ptr<HANDLE>                   mWindowsImpersonationToken;
#endif

    private:
        std::string                             mHttpProxy;
        int                                     mHttpProxyPort;
        TransportProtocol                       mTransportProtocol;
        tstring                                 mUsername;
        tstring                                 mPassword;
        tstring                                 mKerberosSpn;
        bool                                    mEnableCompression;

        CertificatePtr                          mCertificatePtr;
        CertificatePtr                          mCaCertificatePtr;
        CertificateValidationCb                 mCertificateValidationCb;
        tstring                                 mSchannelCertificateValidation;

        std::string                             mOpenSslCipherSuite;

        SslImplementation                       mSslImplementation;
    };

    class CallOptions
    {
    public:
        CallOptions() :
            mAsync(false),
            mRcsSpecified(false), 
            mRcs(Twoway),
            mCallback()
        {}

        CallOptions(RemoteCallSemantics rcs) : 
            mAsync(false),
            mRcsSpecified(true), 
            mRcs(rcs),
            mCallback()
        {}

        CallOptions(RemoteCallSemantics rcs, const boost::function0<void> & callback) : 
            mAsync(true),
            mRcsSpecified(true),
            mRcs(rcs),
            mCallback(callback)
        {}

        CallOptions(boost::function0<void> callback) : 
            mAsync(true),
            mRcsSpecified(false),
            mRcs(Twoway), 
            mCallback(callback)
        {}

        RemoteCallSemantics apply(ClientStub &clientStub) const
        {
            clientStub.setAsync(mAsync);
            clientStub.setAsyncCallback(mCallback);
            return mRcsSpecified ? mRcs : clientStub.getRemoteCallSemantics();
        }

    private:
        bool mAsync;
        bool mRcsSpecified;
        RemoteCallSemantics mRcs;
        boost::function0<void> mCallback;
    };

    class RCF_EXPORT AsyncTwoway : public CallOptions
    {
    public:
        AsyncTwoway(const boost::function0<void> & callback) :
            CallOptions(RCF::Twoway, callback)
        {}
    };

    class RCF_EXPORT AsyncOneway : public CallOptions
    {
    public:
        AsyncOneway(const boost::function0<void> & callback) :
            CallOptions(RCF::Oneway, callback)
        {}
    };

    class RestoreClientTransportGuard
    {
    public:

        RestoreClientTransportGuard(ClientStub &client, ClientStub &clientTemp);
        ~RestoreClientTransportGuard();

    private:
        ClientStub &mClient;
        ClientStub &mClientTemp;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_CLIENTSTUB_HPP
