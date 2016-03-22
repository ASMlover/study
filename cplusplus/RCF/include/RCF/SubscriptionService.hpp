
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

#ifndef INCLUDE_RCF_SUBSCRIPTIONSERVICE_HPP
#define INCLUDE_RCF_SUBSCRIPTIONSERVICE_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/PeriodicTimer.hpp>
#include <RCF/ServerStub.hpp>
#include <RCF/Service.hpp>
#include <RCF/Timer.hpp>

namespace RCF {

    class RcfServer;
    class RcfSession;
    class ClientTransport;
    class ServerTransport;
    class Endpoint;
    class I_RcfClient;

    typedef boost::shared_ptr<I_RcfClient>          RcfClientPtr;
    typedef std::auto_ptr<ClientTransport>          ClientTransportAutoPtr;
    typedef boost::shared_ptr<RcfSession>           RcfSessionPtr;
    typedef boost::weak_ptr<RcfSession>             RcfSessionWeakPtr;
    typedef boost::shared_ptr<ServerTransport>      ServerTransportPtr;

    template<typename T> class RcfClient;
    class I_RequestSubscription;
    class I_Null;

    class SubscriptionService;

    class Subscription;
    typedef boost::shared_ptr<Subscription>         SubscriptionPtr;
    typedef boost::weak_ptr<Subscription>           SubscriptionWeakPtr;

    typedef boost::function1<void, RcfSession &>    OnSubscriptionDisconnect;

    template<typename T>
    class Future;

    class RCF_EXPORT Subscription : boost::noncopyable
    {
    private:

        Subscription(
            SubscriptionService &   subscriptionService,
            ClientTransportAutoPtr  clientTransportAutoPtr,
            RcfSessionWeakPtr       rcfSessionWeakPtr,
            boost::uint32_t         incomingPingIntervalMs,
            const std::string &     publisherUrl,
            const std::string &     topic,
            OnSubscriptionDisconnect onDisconnect);

        void setWeakThisPtr(SubscriptionWeakPtr thisWeakPtr);

    public:

        ~Subscription();

        unsigned int    getPingTimestamp();
        bool            isConnected();
        void            close();
        RcfSessionPtr   getRcfSessionPtr();

    private:
        friend class SubscriptionService;

        static void     onDisconnect(SubscriptionWeakPtr subPtr, RcfSession & session);

        SubscriptionService &       mSubscriptionService;
        SubscriptionWeakPtr         mThisWeakPtr;

        RecursiveMutex              mMutex;
        RcfSessionWeakPtr           mRcfSessionWeakPtr;
        
        boost::shared_ptr<I_RcfClient>  mConnectionPtr;

        boost::uint32_t             mPingIntervalMs;
        bool                        mPingsEnabled;
        std::string                 mPublisherUrl;
        std::string                 mTopic;

        OnSubscriptionDisconnect    mOnDisconnect;
        bool                        mClosed;
    };

    typedef boost::shared_ptr<Subscription> SubscriptionPtr;
    typedef boost::weak_ptr<Subscription> SubscriptionWeakPtr;

    typedef boost::function2<void, SubscriptionPtr, ExceptionPtr> OnAsyncSubscribeCompleted;

    class RCF_EXPORT SubscriptionParms
    {
    public:
        SubscriptionParms();

        void        setTopicName(const std::string & publisherName);
        std::string getTopicName() const;
        void        setPublisherEndpoint(const Endpoint & publisherEp);
        void        setPublisherEndpoint(I_RcfClient & rcfClient);
        void        setOnSubscriptionDisconnect(OnSubscriptionDisconnect onSubscriptionDisconnect);
        void        setOnAsyncSubscribeCompleted(OnAsyncSubscribeCompleted onAsyncSubscribeCompleted);

    private:

        friend class SubscriptionService;

        std::string                             mPublisherName;
        ClientStub                              mClientStub;
        OnSubscriptionDisconnect                mOnDisconnect;
        OnAsyncSubscribeCompleted               mOnAsyncSubscribeCompleted;
    };

    class RCF_EXPORT SubscriptionService :
        public I_Service,
        boost::noncopyable
    {
    public:

        SubscriptionService(boost::uint32_t pingIntervalMs = 0);

        ~SubscriptionService();

        template<typename Interface, typename T>
        SubscriptionPtr createSubscription(
            T & t, 
            const SubscriptionParms & parms)
        {
            std::string defaultPublisherName = getInterfaceName((Interface *) NULL);

            boost::shared_ptr< I_Deref<T> > derefPtr(
                new DerefObj<T>(t));

            RcfClientPtr rcfClientPtr(
                createServerStub((Interface *) 0, (T *) 0, derefPtr));

            return createSubscriptionImpl(rcfClientPtr, parms, defaultPublisherName);
        }

        template<typename Interface, typename T>
        SubscriptionPtr createSubscription(
            T & t, 
            const RCF::Endpoint & publisherEp)
        {
            SubscriptionParms parms;
            parms.setPublisherEndpoint(publisherEp);
            return createSubscription<Interface>(t, parms);
        }

        SubscriptionPtr createSubscriptionImpl(
            RcfClientPtr rcfClientPtr, 
            const SubscriptionParms & parms,
            const std::string & defaultPublisherName);

        void createSubscriptionImplBegin(
            RcfClientPtr rcfClientPtr, 
            const SubscriptionParms & parms,
            const std::string & defaultPublisherName);

        void createSubscriptionImplEnd(
            ExceptionPtr                            ePtr,
            ClientStubPtr                           clientStubPtr,
            boost::int32_t                          ret,
            const std::string &                     publisherName,
            RcfClientPtr                            rcfClientPtr,
            OnSubscriptionDisconnect                onDisconnect,
            OnAsyncSubscribeCompleted               onCompletion,
            boost::uint32_t                         pubToSubPingIntervalMs,
            bool                                    pingsEnabled);

        void closeSubscription(SubscriptionWeakPtr subscriptionPtr);

        void setPingIntervalMs(boost::uint32_t pingIntervalMs);
        boost::uint32_t getPingIntervalMs() const;
        
    private:

        void onServerStart(RcfServer &server);
        void onServerStop(RcfServer &server);

        RcfServer *                     mpServer;
        Mutex                           mSubscriptionsMutex;

        typedef std::set<SubscriptionWeakPtr> Subscriptions;
        Subscriptions                   mSubscriptions;

        boost::uint32_t                 mPingIntervalMs;
        PeriodicTimer                   mPeriodicTimer;

        virtual void onTimer();
        void pingAllSubscriptions();
        void harvestExpiredSubscriptions();

        static void sOnPingCompleted(RecursiveLockPtr lockPtr);

    public:

        SubscriptionPtr onRequestSubscriptionCompleted(
            boost::int32_t                          ret,
            const std::string &                     publisherName,
            ClientStub &                            clientStub,
            RcfClientPtr                            rcfClientPtr,
            OnSubscriptionDisconnect                onDisconnect,
            boost::uint32_t                         pubToSubPingIntervalMs,
            bool                                    pingsEnabled);

    private:

        boost::int32_t doRequestSubscription(
            ClientStub &                    clientStubOrig, 
            const std::string &             publisherName,
            boost::uint32_t                 subToPubPingIntervalMs, 
            boost::uint32_t &               pubToSubPingIntervalMs,
            bool &                          pingsEnabled);

        void doRequestSubscriptionAsync(
            ClientStub &                    clientStubOrig, 
            const std::string &             publisherName,
            RcfClientPtr                    rcfClientPtr,
            const SubscriptionParms &       parms);

        void doRequestSubscriptionAsync_Complete(
            Future<Void>                    fv,
            RcfClientPtr                    requestClientPtr,
            const std::string &             publisherName,
            RcfClientPtr                    rcfClientPtr,
            OnSubscriptionDisconnect        onDisconnect,
            OnAsyncSubscribeCompleted       onCompletion);

        // Legacy subscription requests.

        boost::int32_t doRequestSubscription_Legacy(
            ClientStub &                    clientStubOrig, 
            const std::string &             publisherName,
            boost::uint32_t                 subToPubPingIntervalMs, 
            boost::uint32_t &               pubToSubPingIntervalMs,
            bool &                          pingsEnabled);

        void doRequestSubscriptionAsync_Legacy(
            ClientStub &                    clientStubOrig, 
            const std::string &             publisherName,
            RcfClientPtr                    rcfClientPtr,
            const SubscriptionParms &       parms);

        void doRequestSubscriptionAsync_Legacy_Complete(
            ClientStubPtr                   clientStubPtr,
            Future<boost::int32_t>          fRet,
            const std::string &             publisherName,
            RcfClientPtr                    rcfClientPtr,
            OnSubscriptionDisconnect        onDisconnect,
            OnAsyncSubscribeCompleted       onCompletion,
            Future<boost::uint32_t>         pubToSubPingIntervalMs,
            bool                            pingsEnabled);

    };

    typedef boost::shared_ptr<SubscriptionService> SubscriptionServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_SUBSCRIPTIONSERVICE_HPP
