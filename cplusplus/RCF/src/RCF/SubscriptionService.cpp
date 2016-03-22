
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

#include <RCF/SubscriptionService.hpp>

#include <boost/bind.hpp>

#include <typeinfo>

#include <RCF/AsioFwd.hpp>
#include <RCF/AsioServerTransport.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Future.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>

namespace RCF {

    SubscriptionParms::SubscriptionParms() : mClientStub("")
    {
    }

    void SubscriptionParms::setTopicName(const std::string & publisherName)
    {
        mPublisherName = publisherName;
    }

    std::string SubscriptionParms::getTopicName() const
    {
        return mPublisherName;
    }

    void SubscriptionParms::setPublisherEndpoint(const Endpoint & publisherEp)
    {
        mClientStub.setEndpoint(publisherEp);
    }

    void SubscriptionParms::setPublisherEndpoint(I_RcfClient & rcfClient)
    {
        mClientStub = rcfClient.getClientStub();
        mClientStub.setTransport( rcfClient.getClientStub().releaseTransport() );
    }

    void SubscriptionParms::setOnSubscriptionDisconnect(OnSubscriptionDisconnect onSubscriptionDisconnect)
    {
        mOnDisconnect = onSubscriptionDisconnect;
    }

    void SubscriptionParms::setOnAsyncSubscribeCompleted(OnAsyncSubscribeCompleted onAsyncSubscribeCompleted)
    {
        mOnAsyncSubscribeCompleted = onAsyncSubscribeCompleted;
    }

    Subscription::~Subscription()
    {
        RCF_DTOR_BEGIN
            close();
        RCF_DTOR_END
    }

    void Subscription::setWeakThisPtr(SubscriptionWeakPtr thisWeakPtr)
    {
        mThisWeakPtr = thisWeakPtr;
    }

    bool Subscription::isConnected()
    {
        RecursiveLock lock(mMutex);
        if ( !mConnectionPtr )
        {
            return true;
        }
        else
        {
            return mConnectionPtr->getClientStub().isConnected();
        }
    }

    unsigned int Subscription::getPingTimestamp()
    {
        RcfSessionPtr rcfSessionPtr;
        {
            RecursiveLock lock(mMutex);
            rcfSessionPtr = mRcfSessionWeakPtr.lock();
        }
        if (rcfSessionPtr)
        {
            return rcfSessionPtr->getPingTimestamp();
        }
        return 0;
    }

    void Subscription::close()
    {
        RCF_ASSERT(mThisWeakPtr != SubscriptionWeakPtr());

        {
            RecursiveLock lock(mMutex);

            if (mClosed)
            {
                return;
            }

            RcfSessionPtr rcfSessionPtr(mRcfSessionWeakPtr.lock());
            if (rcfSessionPtr)
            {

                // When this function returns, the caller is allowed to delete
                // the object that this subscription refers to. So we need to
                // make sure there are no calls in progress.

                Lock sessionLock(rcfSessionPtr->mStopCallInProgressMutex);
                rcfSessionPtr->mStopCallInProgress = true;

                // Remove subscription binding.
                rcfSessionPtr->setDefaultStubEntryPtr(StubEntryPtr());

                // Clear the destroy callback.
                // TODO: how do we know that we're not clearing someone else's callback?
                rcfSessionPtr->setOnDestroyCallback(
                    RcfSession::OnDestroyCallback());
            }

            mRcfSessionWeakPtr.reset();
            
            if ( mConnectionPtr )
            {
                mConnectionPtr->getClientStub().disconnect();
            }

            mClosed = true;
        }

        mSubscriptionService.closeSubscription(mThisWeakPtr);
    }

    RcfSessionPtr Subscription::getRcfSessionPtr()
    {
        RecursiveLock lock(mMutex);
        return mRcfSessionWeakPtr.lock();
    }

    void Subscription::onDisconnect(SubscriptionWeakPtr subWeakPtr, RcfSession & session)
    {
        SubscriptionPtr subPtr = subWeakPtr.lock();
        if (subPtr)
        {
            OnSubscriptionDisconnect f = subPtr->mOnDisconnect;

            subPtr->close();

            if (f)
            {
                f(session);
            }
        }
    }

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 ) // warning C4355: 'this' : used in base member initializer list
#endif

    SubscriptionService::SubscriptionService(boost::uint32_t pingIntervalMs) :
        mpServer(),
        mPingIntervalMs(pingIntervalMs),
        mPeriodicTimer(*this, pingIntervalMs)
    {}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    SubscriptionService::~SubscriptionService()
    {
    }

    SubscriptionPtr SubscriptionService::onRequestSubscriptionCompleted(
        boost::int32_t                      ret,
        const std::string &                 publisherName,
        ClientStub &                        clientStub,
        RcfClientPtr                        rcfClientPtr,
        OnSubscriptionDisconnect            onDisconnect,
        boost::uint32_t                     pubToSubPingIntervalMs,
        bool                                pingsEnabled)
    {
        if (ret != RcfError_Ok)
        {
            RCF_THROW( Exception( Error(ret) ) );
        }

        ClientTransportAutoPtr clientTransportAutoPtr( 
                clientStub.releaseTransport() );

        ServerTransport * pTransport = NULL;
        ServerTransportEx * pTransportEx = NULL;

        pTransport = & mpServer->findTransportCompatibleWith(
            *clientTransportAutoPtr);

        pTransportEx = dynamic_cast<ServerTransportEx *>(pTransport);

        ServerTransportEx & serverTransportEx = * pTransportEx; 

        SessionPtr sessionPtr = serverTransportEx.createServerSession(
            clientTransportAutoPtr,
            StubEntryPtr(new StubEntry(rcfClientPtr)),
            true);

        RCF_ASSERT( sessionPtr );

        RcfSessionPtr rcfSessionPtr = sessionPtr;

        rcfSessionPtr->setUserData(clientStub.getUserData());
        rcfSessionPtr->setPingTimestamp();

        std::string publisherUrl;
        EndpointPtr epPtr = clientStub.getEndpoint();
        if (epPtr)
        {
            publisherUrl = epPtr->asString();
        }

        //if (!clientTransportAutoPtr->isAssociatedWithIoService())
        //{
        //    AsioServerTransport & asioTransport = dynamic_cast<AsioServerTransport &>(
        //        mpServer->getServerTransport());

        //    clientTransportAutoPtr->associateWithIoService(asioTransport.getIoService());
        //}

        SubscriptionPtr subscriptionPtr( new Subscription(
            *this,
            clientTransportAutoPtr, 
            rcfSessionPtr, 
            pubToSubPingIntervalMs, 
            publisherUrl,
            publisherName,
            onDisconnect));

        rcfSessionPtr->setOnDestroyCallback( boost::bind(
            &Subscription::onDisconnect,
            SubscriptionWeakPtr(subscriptionPtr),
            _1));

        subscriptionPtr->setWeakThisPtr(subscriptionPtr);

        subscriptionPtr->mPingsEnabled = pingsEnabled;

        Lock lock(mSubscriptionsMutex);
        mSubscriptions.insert(subscriptionPtr);

        return subscriptionPtr;                
    }


    boost::int32_t SubscriptionService::doRequestSubscription(
        ClientStub &            clientStubOrig, 
        const std::string &     publisherName,
        boost::uint32_t subToPubPingIntervalMs, 
        boost::uint32_t &       pubToSubPingIntervalMs,
        bool &                  pingsEnabled)
    {
        I_RcfClient client("", clientStubOrig);
        ClientStub & clientStub = client.getClientStub();
        clientStub.setTransport(clientStubOrig.releaseTransport());

        pingsEnabled = true;

        // Set OOB request.
        OobRequestSubscription msg(
            clientStubOrig.getRuntimeVersion(), 
            publisherName, 
            subToPubPingIntervalMs);

        ByteBuffer controlRequest;
        msg.encodeRequest(controlRequest);
        clientStub.setOutofBandRequest(controlRequest);

        clientStub.ping(RCF::Twoway);

        // Get OOB response.
        ByteBuffer controlResponse = clientStub.getOutOfBandResponse();
        clientStub.setOutofBandRequest(ByteBuffer());
        clientStub.setOutofBandResponse(ByteBuffer());
        msg.decodeResponse(controlResponse);

        boost::int32_t ret = msg.mResponseError;
        pubToSubPingIntervalMs = msg.mPubToSubPingIntervalMs;

        clientStubOrig.setTransport( client.getClientStub().releaseTransport() );

        return ret;
    }

    SubscriptionPtr SubscriptionService::createSubscriptionImpl(
        RcfClientPtr rcfClientPtr, 
        const SubscriptionParms & parms,
        const std::string & defaultPublisherName)
    {
        if (parms.mOnAsyncSubscribeCompleted)
        {
            // Async code path.
            createSubscriptionImplBegin(rcfClientPtr, parms, defaultPublisherName);
            return SubscriptionPtr();
        }

        ClientStub & clientStub = const_cast<ClientStub &>(parms.mClientStub);
        OnSubscriptionDisconnect onDisconnect = parms.mOnDisconnect;
        std::string publisherName = parms.mPublisherName;
        if (publisherName.empty())
        {
            publisherName = defaultPublisherName;
        }

        boost::uint32_t     subToPubPingIntervalMs = mPingIntervalMs;
        boost::uint32_t     pubToSubPingIntervalMs = 0;
        bool                pingsEnabled = true;

        boost::int32_t ret = 0;

        // First round trip, to do version negotiation with the server.
        clientStub.ping();

        if (clientStub.getRuntimeVersion() <= 11)
        {
            ret = doRequestSubscription_Legacy(
                clientStub,
                publisherName,
                subToPubPingIntervalMs,
                pubToSubPingIntervalMs,
                pingsEnabled);
        }
        else
        {
            ret = doRequestSubscription(
                clientStub,
                publisherName,
                subToPubPingIntervalMs,
                pubToSubPingIntervalMs,
                pingsEnabled);
        }

        SubscriptionPtr subscriptionPtr = onRequestSubscriptionCompleted(
            ret,
            publisherName,
            clientStub,
            rcfClientPtr,
            onDisconnect,
            pubToSubPingIntervalMs,
            pingsEnabled);

        return subscriptionPtr;
    }

    void SubscriptionService::createSubscriptionImplEnd(
        ExceptionPtr                    ePtr,
        ClientStubPtr                   clientStubPtr,
        boost::int32_t                  ret,
        const std::string &             publisherName,
        RcfClientPtr                    rcfClientPtr,
        OnSubscriptionDisconnect        onDisconnect,
        OnAsyncSubscribeCompleted       onCompletion,
        boost::uint32_t                 incomingPingIntervalMs,
        bool                            pingsEnabled)
    {
        SubscriptionPtr subscriptionPtr;

        if (!ePtr && ret != RcfError_Ok)
        {
            ePtr.reset( new Exception( Error(ret) ) );
        }

        if (!ePtr)
        {
            subscriptionPtr = onRequestSubscriptionCompleted(
                ret,
                publisherName,
                *clientStubPtr,
                rcfClientPtr,
                onDisconnect,
                incomingPingIntervalMs,
                pingsEnabled);
        }

        onCompletion(subscriptionPtr, ePtr);
    }

    void SubscriptionService::doRequestSubscriptionAsync_Complete(
        Future<Void>                    fv,
        RcfClientPtr                    requestClientPtr,
        const std::string &             publisherName,
        RcfClientPtr                    rcfClientPtr,
        OnSubscriptionDisconnect        onDisconnect,
        OnAsyncSubscribeCompleted       onCompletion)
    {
        bool pingsEnabled = true;

        boost::uint32_t ret = 0;
        boost::uint32_t pubToSubPingIntervalMs = 0;

        ExceptionPtr ePtr( fv.getAsyncException().release() );
        if (!ePtr)
        {
            // Get OOB response.
            ClientStub & stub = requestClientPtr->getClientStub();
            OobRequestSubscription msg(stub.getRuntimeVersion());
            ByteBuffer controlResponse = stub.getOutOfBandResponse();
            stub.setOutofBandRequest(ByteBuffer());
            stub.setOutofBandResponse(ByteBuffer());
            msg.decodeResponse(controlResponse);

            ret = msg.mResponseError; 
            pubToSubPingIntervalMs = msg.mPubToSubPingIntervalMs;
        }

        createSubscriptionImplEnd(
            ePtr, 
            requestClientPtr->getClientStubPtr(), 
            ret, 
            publisherName, 
            rcfClientPtr, 
            onDisconnect, 
            onCompletion, 
            pubToSubPingIntervalMs, 
            pingsEnabled);
    }

    void SubscriptionService::doRequestSubscriptionAsync(
        ClientStub &            clientStubOrig, 
        const std::string &     publisherName,
        RcfClientPtr            rcfClientPtr,
        const SubscriptionParms & parms)
    {
        RcfClientPtr requestClientPtr( new I_RcfClient("", clientStubOrig) );
        requestClientPtr->getClientStub().setTransport( clientStubOrig.releaseTransport() );
        requestClientPtr->getClientStub().setAsyncDispatcher(*mpServer);

        // Set OOB request.
        boost::uint32_t subToPubPingIntervalMs = mPingIntervalMs;
        OobRequestSubscription msg(
            clientStubOrig.getRuntimeVersion(), 
            publisherName, 
            subToPubPingIntervalMs);

        ByteBuffer controlRequest;
        msg.encodeRequest(controlRequest);
        requestClientPtr->getClientStub().setOutofBandRequest(controlRequest);

        Future<Void> fv;
        fv = requestClientPtr->getClientStub().ping( RCF::AsyncTwoway( boost::bind(
            &SubscriptionService::doRequestSubscriptionAsync_Complete,
            this,
            fv,
            requestClientPtr,
            publisherName,
            rcfClientPtr,
            parms.mOnDisconnect,
            parms.mOnAsyncSubscribeCompleted )));
    }

    void SubscriptionService::createSubscriptionImplBegin(
        RcfClientPtr rcfClientPtr, 
        const SubscriptionParms & parms,
        const std::string & defaultPublisherName)
    {
        ClientStub & clientStub = const_cast<ClientStub &>(parms.mClientStub);
        OnSubscriptionDisconnect onDisconnect = parms.mOnDisconnect;
        std::string publisherName = parms.mPublisherName;
        OnAsyncSubscribeCompleted onCompletion = parms.mOnAsyncSubscribeCompleted;

        if (publisherName.empty())
        {
            publisherName = defaultPublisherName;
        }
        
        RCF_ASSERT(onCompletion);

        if (clientStub.getRuntimeVersion() <= 11)
        {
            doRequestSubscriptionAsync_Legacy(
                clientStub, 
                publisherName, 
                rcfClientPtr,
                parms);
        }
        else
        {
            doRequestSubscriptionAsync(
                clientStub, 
                publisherName, 
                rcfClientPtr,
                parms);
        }
    }

    void SubscriptionService::closeSubscription(
        SubscriptionWeakPtr subscriptionWeakPtr)
    {
        Lock lock(mSubscriptionsMutex);
        mSubscriptions.erase(subscriptionWeakPtr);
    }

    void SubscriptionService::setPingIntervalMs(boost::uint32_t pingIntervalMs)
    {
        mPingIntervalMs = pingIntervalMs;
    }

    boost::uint32_t SubscriptionService::getPingIntervalMs() const
    {
        return mPingIntervalMs;
    }

    void SubscriptionService::onServerStart(RcfServer &server)
    {
        mpServer = &server;
        mPeriodicTimer.setIntervalMs(mPingIntervalMs);
        mPeriodicTimer.start();
    }

    void SubscriptionService::onServerStop(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);

        mPeriodicTimer.stop();

        Subscriptions subs;

        {
            Lock writeLock(mSubscriptionsMutex);
            subs = mSubscriptions;
        }

        for (Subscriptions::iterator iter = subs.begin();
            iter != subs.end();
            ++iter)
        {
            SubscriptionPtr subscriptionPtr = iter->lock();
            if (subscriptionPtr)
            {
                subscriptionPtr->close();
            }
        }

        {
            Lock writeLock(mSubscriptionsMutex);
            RCF_ASSERT(mSubscriptions.empty());
        }

        mSubscriptions.clear();
        subs.clear();

        mpServer = NULL;
    }

    void SubscriptionService::onTimer()
    {
        pingAllSubscriptions();
        harvestExpiredSubscriptions();
    }

    void SubscriptionService::sOnPingCompleted(RecursiveLockPtr lockPtr)
    {
        lockPtr->unlock();
        lockPtr.reset();
    }

    void SubscriptionService::pingAllSubscriptions()
    {
        // Send oneway pings on all our subscriptions, so the publisher
        // knows we're still alive.

        Subscriptions subs;
        {
            Lock lock(mSubscriptionsMutex);
            subs = mSubscriptions;
        }

        Subscriptions::iterator iter;
        for (iter = subs.begin(); iter != subs.end(); ++iter)
        {
            SubscriptionPtr subPtr = iter->lock();
            if (subPtr)
            {
                Subscription & sub = * subPtr;
                if ( sub.mPingsEnabled && sub.mConnectionPtr && sub.isConnected() )
                {
                    // Lock will be unlocked when the asynchronous send completes.
                    // Using recursive lock here because the ping may result in a 
                    // disconnect, which will then automatically close the connection
                    // and close the subscription, which requires the lock to be taken again.
                    boost::shared_ptr<RecursiveLock> lockPtr( new RecursiveLock(sub.mMutex) );

                    // TODO: async pings
                    bool asyncPings = false;
                    if (asyncPings)
                    {
                        AsioErrorCode ecDummy;

                        sub.mConnectionPtr->getClientStub().ping(
                            RCF::AsyncOneway(boost::bind(
                                &SubscriptionService::sOnPingCompleted, 
                                lockPtr)));
                    }
                    else
                    {
                        try
                        {
                            sub.mConnectionPtr->getClientStub().ping(RCF::Oneway);
                        }
                        catch(const RCF::Exception & e)
                        {
                            std::string errMsg = e.getErrorString();
                            RCF_UNUSED_VARIABLE(errMsg);
                        }
                    }
                }
            }
        }
    }

    void SubscriptionService::harvestExpiredSubscriptions()
    {
        // Kill off subscriptions that haven't received any recent pings.

        std::vector<SubscriptionPtr> subsToDrop;

        {
            Lock lock(mSubscriptionsMutex);

            Subscriptions::iterator iter;
            for (iter = mSubscriptions.begin(); iter != mSubscriptions.end(); ++iter)
            {
                SubscriptionPtr subPtr = iter->lock();
                if (subPtr)
                {
                    Subscription & sub = * subPtr;

                    RecursiveLock subscriptionLock(sub.mMutex);
                    RcfSessionPtr sessionPtr = sub.mRcfSessionWeakPtr.lock();

                    if (!sessionPtr)
                    {
                        RCF_LOG_2()(sub.mPublisherUrl)(sub.mTopic) << "Dropping subscription. Publisher has closed connection.";
                        subsToDrop.push_back(subPtr);
                    }
                    else if (sub.mPingsEnabled)
                    {
                        boost::uint32_t pingIntervalMs = sub.mPingIntervalMs;
                        if (pingIntervalMs)
                        {
                            RCF::Timer pingTimer(sessionPtr->getPingTimestamp());
                            if (pingTimer.elapsed(5000 + 2*pingIntervalMs))
                            {
                                RCF_LOG_2()(sub.mPublisherUrl)(sub.mTopic)(sub.mPingIntervalMs) << "Dropping subscription. Publisher has not sent pings.";
                                subsToDrop.push_back(subPtr);
                            }
                        }
                    }
                }
            }

            for (std::size_t i=0; i<subsToDrop.size(); ++i)
            {
                mSubscriptions.erase( subsToDrop[i] );
            }
        }

        subsToDrop.clear();
    }

    Subscription::Subscription(
        SubscriptionService & subscriptionService,
        ClientTransportAutoPtr clientTransportAutoPtr,
        RcfSessionWeakPtr rcfSessionWeakPtr,
        boost::uint32_t incomingPingIntervalMs,
        const std::string & publisherUrl,
        const std::string & topic,
        OnSubscriptionDisconnect onDisconnect) :
            mSubscriptionService(subscriptionService),
            mRcfSessionWeakPtr(rcfSessionWeakPtr),
            mConnectionPtr(),
            mPingIntervalMs(incomingPingIntervalMs),
            mPingsEnabled(false),
            mPublisherUrl(publisherUrl),
            mTopic(topic),
            mOnDisconnect(onDisconnect),
            mClosed(false)
    {
        if ( clientTransportAutoPtr.get() )
        {
            mConnectionPtr.reset(new I_RcfClient("", clientTransportAutoPtr));
            mConnectionPtr->getClientStub().setAutoReconnect(false);
        }
    }
   
} // namespace RCF
