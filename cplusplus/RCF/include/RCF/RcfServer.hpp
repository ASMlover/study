
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

#ifndef INCLUDE_RCF_RCFSERVER_HPP
#define INCLUDE_RCF_RCFSERVER_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/weak_ptr.hpp>

#include <RCF/CheckRtti.hpp>
#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/ServerObjectService.hpp>
#include <RCF/ServerStub.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadPool.hpp>

#if RCF_FEATURE_FILETRANSFER==1
#include <RCF/FileTransferService.hpp>
#endif

#if RCF_FEATURE_PUBSUB==1
#include <RCF/PublishingService.hpp>
#include <RCF/SubscriptionService.hpp>
#endif

namespace RCF {

    class ServerTransport;
    class StubEntry;
    class I_Service;
    class RcfSession;
    class Endpoint;
    class I_FilterFactoryLookupProvider;
    class I_RcfClient;
    class IpServerTransport;
    class PingBackService;
    class FileTransferService;
    class ObjectFactoryService;
    class FilterService;
    class SessionTimeoutService;
    class PublishingService;
    class SubscriptionService;
    class SessionObjectFactoryService;
    class ObjectFactoryService;
    class CallbackConnectionService;
    
    typedef boost::shared_ptr<ServerTransport>                  ServerTransportPtr;
    typedef boost::shared_ptr<StubEntry>                        StubEntryPtr;
    typedef boost::shared_ptr<I_Service>                        ServicePtr;
    typedef boost::shared_ptr<RcfSession>                       RcfSessionPtr;
    typedef boost::shared_ptr<I_FilterFactoryLookupProvider>    FilterFactoryLookupProviderPtr;
    typedef boost::shared_ptr<I_RcfClient>                      RcfClientPtr;
    typedef boost::shared_ptr<Endpoint>                         EndpointPtr;
    typedef boost::shared_ptr<PingBackService>                  PingBackServicePtr;
    typedef boost::shared_ptr<FileTransferService>              FileTransferServicePtr;
    typedef boost::shared_ptr<ObjectFactoryService>             ObjectFactoryServicePtr;
    typedef boost::shared_ptr<FilterService>                    FilterServicePtr;
    typedef boost::shared_ptr<SessionTimeoutService>            SessionTimeoutServicePtr;
    typedef boost::shared_ptr<PublishingService>                PublishingServicePtr;
    typedef boost::shared_ptr<SubscriptionService>              SubscriptionServicePtr;
    typedef boost::shared_ptr<SessionObjectFactoryService>      SessionObjectFactoryServicePtr;
    typedef boost::shared_ptr<ObjectFactoryService>             ObjectFactoryServicePtr;
    typedef boost::shared_ptr<CallbackConnectionService>        CallbackConnectionServicePtr;
    typedef boost::weak_ptr<RcfSession>                         RcfSessionWeakPtr;  

    template<typename Interface>
    class Publisher;

    class Subscription;
    typedef boost::shared_ptr<Subscription> SubscriptionPtr;

    class SubscriptionParms;

    class BandwidthQuota;
    typedef boost::shared_ptr<BandwidthQuota> BandwidthQuotaPtr;

    class FileDownloadInfo;
    class FileUploadInfo;

    class JsonRpcRequest;
    class JsonRpcResponse;

    class SspiFilter;

    class HttpSession;
    typedef boost::shared_ptr<HttpSession> HttpSessionPtr;

    typedef boost::function2<void, RcfSessionPtr, ClientTransportAutoPtr> OnCallbackConnectionCreated;

    class RCF_EXPORT RcfServer : boost::noncopyable
    {
    public:

        RcfServer();
        RcfServer(const Endpoint &endpoint);
        RcfServer(ServicePtr servicePtr);
        RcfServer(ServerTransportPtr serverTransportPtr);

        ~RcfServer();

        typedef boost::function0<void> JoinFunctor;
        typedef boost::function1<void, RcfServer&> StartCallback;

        void    start();
        void    stop();

    private:

        void init();

    public:

        ServerTransport &     
                getServerTransport();

        I_Service &             
                getServerTransportService();

        ServerTransportPtr      
                getServerTransportPtr();

        IpServerTransport &   
                getIpServerTransport();

    private:

        bool    addService(
                    ServicePtr servicePtr);

        bool    removeService(
                    ServicePtr servicePtr);

    public:

        PingBackServicePtr 
                getPingBackServicePtr();

        FileTransferServicePtr
                getFileTransferServicePtr();

        SessionTimeoutServicePtr 
                getSessionTimeoutServicePtr();

        ObjectFactoryServicePtr
                getObjectFactoryServicePtr();

        SessionObjectFactoryServicePtr
                getSessionObjectFactoryServicePtr();

        PublishingServicePtr
                getPublishingServicePtr();

        SubscriptionServicePtr
                getSubscriptionServicePtr();

        FilterServicePtr getFilterServicePtr();

        bool    addServerTransport(
                    ServerTransportPtr serverTransportPtr);

        bool    removeServerTransport(
                    ServerTransportPtr serverTransportPtr);

        ServerTransport & 
                findTransportCompatibleWith(ClientTransport & clienetTransport);
        
        void    setStartCallback(const StartCallback &startCallback);       
        
    private:
        void    invokeStartCallback();

    private:
        ServerBindingPtr bindImpl(
                    const std::string &name, 
                    RcfClientPtr rcfClientPtr);

        //*************************************
        // async io transport interface

    //private:
    public:
        SessionPtr  createSession();
        void        onReadCompleted(SessionPtr sessionPtr);
        void        onWriteCompleted(SessionPtr sessionPtr);


        //*************************************
        // transports, queues and threads

    private:

        void startImpl();

    public:


        template<typename Iter>
        void enumerateSessions(const Iter & iter)
        {

            for (std::size_t i=0; i<mServerTransports.size(); ++i)
            {
                mServerTransports[i]->enumerateSessions(iter);
            }

        }

        //*************************************
        // stub management

    private:
        ReadWriteMutex                                  mStubMapMutex;
        typedef std::map<std::string, StubEntryPtr>     StubMap;
        StubMap                                         mStubMap;

        typedef boost::function<void(const JsonRpcRequest &, JsonRpcResponse &)> JsonRpcMethod;
        typedef std::map<std::string, JsonRpcMethod>    JsonRpcMethods;
        JsonRpcMethods                                  mJsonRpcMethods;

        friend class RcfSession;



        //*************************************
        // service management

    private:
        std::vector<ServerTransportPtr>                 mServerTransports;
        std::vector<ServicePtr>                         mServices;
        FilterServicePtr                                mFilterServicePtr;
        PingBackServicePtr                              mPingBackServicePtr;
        FileTransferServicePtr                          mFileTransferServicePtr;
        SessionTimeoutServicePtr                        mSessionTimeoutServicePtr;
        PublishingServicePtr                            mPublishingServicePtr;
        SubscriptionServicePtr                          mSubscriptionServicePtr;
        CallbackConnectionServicePtr                    mCallbackConnectionServicePtr;
        SessionObjectFactoryServicePtr                  mSessionObjectFactoryServicePtr;
        ObjectFactoryServicePtr                         mObjectFactoryServicePtr;
        ServerObjectServicePtr                          mServerObjectServicePtr;

        void startService(ServicePtr servicePtr) const;
        void stopService(ServicePtr servicePtr) const;
        void resolveServiceThreadPools(ServicePtr servicePtr) const;

        friend class AsioNetworkSession;
        FilterPtr createFilter(int filterId);

    private:
        // start/stop functionality
        StartCallback                                   mStartCallback;
        Condition                                       mStartEvent;
        Condition                                       mStopEvent;

        Mutex                                           mStartStopMutex;
        bool                                            mStarted;


    public:
        void                    setThreadPool(ThreadPoolPtr threadPoolPtr);
        ThreadPoolPtr           getThreadPool();
        ServerTransport &     addEndpoint(const Endpoint & endpoint);

    private:
        ThreadPoolPtr                                   mThreadPoolPtr;

    public:

        void                    waitForStopEvent();
        void                    waitForStartEvent();

    bool                        isStarted();

        // TODO: get rid of this hack
    private:
        friend class MethodInvocationRequest;

    public:
        boost::uint32_t         getRuntimeVersion();
        void                    setRuntimeVersion(boost::uint32_t version);

        boost::uint32_t         getArchiveVersion();
        void                    setArchiveVersion(boost::uint32_t version);

    private:
        boost::uint32_t                                 mRuntimeVersion;
        boost::uint32_t                                 mArchiveVersion;


    public:

        template<typename I1, typename ImplementationT> 
        ServerBindingPtr bind(ImplementationT & t, const std::string &name = "")
        { 
            boost::shared_ptr< I_Deref<ImplementationT> > derefPtr( 
                new DerefObj<ImplementationT>(t) ); 

            RcfClientPtr rcfClientPtr = createServerStub( 
                (I1 *) 0, 
                (ImplementationT *) 0, derefPtr);

            return bindImpl( 
                name.empty() ? 
                    I1::getInterfaceName() : 
                    name , 
                rcfClientPtr); 
        }

        template<typename InterfaceT>
        bool unbind(const std::string &name_ = "")
        {
            const std::string &name = (name_ == "") ?
                getInterfaceName((InterfaceT *) NULL) :
                name_;

            WriteLock writeLock(mStubMapMutex);
            mStubMap.erase(name);
            return true;
        }

#if RCF_FEATURE_JSON==1
        void bindJsonRpc(JsonRpcMethod jsonRpcMethod, const std::string & jsonRpcName);
        void unbindJsonRpc(const std::string & jsonRpcName);
#endif
        
        void setSupportedTransportProtocols(
            const std::vector<TransportProtocol> & protocols);

        const std::vector<TransportProtocol> & 
            getSupportedTransportProtocols() const;

        void setCertificate(CertificatePtr certificatePtr);
        CertificatePtr getCertificate();

        void setOpenSslCipherSuite(const std::string & cipherSuite);
        std::string getOpenSslCipherSuite() const;

        void setCaCertificate(CertificatePtr certificatePtr);
        CertificatePtr getCaCertificate();


        typedef boost::function<bool(Certificate *)> CertificateValidationCb;
        void setCertificateValidationCallback(CertificateValidationCb certificateValidationCb);
        const CertificateValidationCb & getCertificateValidationCallback() const;

        void setEnableSchannelCertificateValidation(const tstring & peerName);
        tstring getEnableSchannelCertificateValidation() const;

        void setSslImplementation(SslImplementation sslImplementation);
        SslImplementation getSslImplementation() const;

        void setSessionTimeoutMs(boost::uint32_t sessionTimeoutMs);
        boost::uint32_t getSessionTimeoutMs();

        void setSessionHarvestingIntervalMs(boost::uint32_t sessionHarvestingIntervalMs);
        boost::uint32_t getSessionHarvestingIntervalMs();

        void setHttpSessionTimeoutMs(boost::uint32_t httpSessionTimeoutMs);
        boost::uint32_t getHttpSessionTimeoutMs();

        void setOnCallbackConnectionCreated(OnCallbackConnectionCreated onCallbackConnectionCreated);
        OnCallbackConnectionCreated getOnCallbackConnectionCreated();

        void setOfsMaxNumberOfObjects(boost::uint32_t ofsMaxNumberOfObjects);
        void setOfsObjectTimeoutS(boost::uint32_t ofsObjectTimeoutS);
        void setOfsCleanupIntervalS(boost::uint32_t ofsCleanupIntervalS);
        void setOfsCleanupThreshold(float ofsCleanupThreshold);

        boost::uint32_t getOfsMaxNumberOfObjects() const;
        boost::uint32_t  getOfsObjectTimeoutS() const;
        boost::uint32_t getOfsCleanupIntervalS() const;
        float getOfsCleanupThreshold() const;

#if RCF_FEATURE_FILETRANSFER==1

        typedef FileTransferService::OnFileDownloadProgress OnFileDownloadProgress;
        typedef FileTransferService::OnFileUploadProgress OnFileUploadProgress;

        void setOnFileDownloadProgress(OnFileDownloadProgress onFileDownloadProgress);
        void setOnFileUploadProgress(OnFileUploadProgress onFileUploadProgress);

        void setFileUploadDirectory(const std::string & uploadDir);
        std::string getFileUploadDirectory() const;

        typedef boost::function1<BandwidthQuotaPtr, RcfSession &> BandwidthQuotaCallback;
        typedef BandwidthQuotaCallback FileUploadQuotaCallback;
        typedef BandwidthQuotaCallback FileDownloadQuotaCallback;

        void setFileUploadBandwidthLimit(boost::uint32_t uploadQuotaBps);
        boost::uint32_t getFileUploadBandwidthLimit() const;

        void setFileUploadCustomBandwidthLimit(FileUploadQuotaCallback uploadQuotaCb);

        void setFileDownloadBandwidthLimit(boost::uint32_t downloadQuotaBps);
        boost::uint32_t getFileDownloadBandwidthLimit() const;

        void setFileDownloadCustomBandwidthLimit(FileDownloadQuotaCallback downloadQuotaCb);

    private:

        OnFileDownloadProgress          mOnFileDownloadProgress;
        OnFileUploadProgress            mOnFileUploadProgress;

        std::string                     mFileUploadDirectory;

        boost::uint32_t                 mFileUploadQuota;
        FileUploadQuotaCallback         mFileUploadQuotaCb;

        boost::uint32_t                 mFileDownloadQuota;
        FileDownloadQuotaCallback       mFileDownloadQuotaCb;

        friend class FileTransferService;

#endif

    private:

        mutable ReadWriteMutex          mPropertiesMutex;

        std::vector<TransportProtocol>  mSupportedProtocols;
        CertificatePtr                  mCertificatePtr;
        std::string                     mOpenSslCipherSuite;

        CertificatePtr                  mCaCertificatePtr;
        CertificateValidationCb         mCertificateValidationCb;
        tstring                         mSchannelCertificateValidation;

        SslImplementation               mSslImplementation;

        boost::uint32_t                 mSessionTimeoutMs;
        boost::uint32_t                 mSessionHarvestingIntervalMs;

        boost::uint32_t                 mHttpSessionTimeoutMs;

        OnCallbackConnectionCreated     mOnCallbackConnectionCreated;

        boost::uint32_t                 mOfsMaxNumberOfObjects;
        boost::uint32_t                 mOfsObjectTimeoutS;
        boost::uint32_t                 mOfsCleanupIntervalS;
        float                           mOfsCleanupThreshold;

    public:

#if RCF_FEATURE_PUBSUB==1

        template<typename Interface>
        boost::shared_ptr< Publisher<Interface> > createPublisher()
        {
            PublisherParms parms;
            return mPublishingServicePtr->createPublisher<Interface>(parms);
        }

        template<typename Interface>
        boost::shared_ptr< Publisher<Interface> > createPublisher(
            const PublisherParms & parms)
        {
            return mPublishingServicePtr->createPublisher<Interface>(parms);
        }

        template<typename Interface, typename T>
        boost::shared_ptr< Subscription > createSubscription(
            T & t, 
            const RCF::Endpoint & publisherEp)
        {
            RCF_ASSERT(mStarted);
            SubscriptionParms parms;
            parms.setPublisherEndpoint(publisherEp);
            return mSubscriptionServicePtr->createSubscription<Interface>(t, publisherEp);
        }

        template<typename Interface, typename T>
        boost::shared_ptr< Subscription > createSubscription(
            T & t, 
            const SubscriptionParms & parms)
        {
            RCF_ASSERT(mStarted);
            return mSubscriptionServicePtr->createSubscription<Interface>(t, parms);
        }

#endif

    private:

        boost::uint32_t mServerObjectHarvestingIntervalS;

        Mutex                                   mHttpSessionMapMutex;
        std::map<std::string, HttpSessionPtr>   mHttpSessionMap;

        friend class HttpSessionFilter;

        HttpSessionPtr  attachHttpSession(const std::string & httpSessionId, bool allowCreate, ExceptionPtr & ePtr);
        void            detachHttpSession(HttpSessionPtr httpSessionPtr);

        friend class ServerObjectService;
        void            harvestHttpSessions();

    public:

        boost::uint32_t getServerObjectHarvestingIntervalS() const;
        void setServerObjectHarvestingIntervalS(boost::uint32_t harvestingIntervalS);

        template<typename T>
        boost::shared_ptr<T> queryServerObject(
            const std::string & objectKey)
        {
            return mServerObjectServicePtr->queryServerObject<T>(objectKey);
        }

        template<typename T>
        boost::shared_ptr<T> getServerObject(
            const std::string & objectKey, 
            boost::uint32_t timeoutMs)
        {
            return mServerObjectServicePtr->getServerObject<T>(objectKey, timeoutMs);
        }

        void deleteServerObject(const std::string & objectKey);

        StubEntryPtr findStubEntryForToken(const Token & token);
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_RCFSERVER_HPP

