
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

#include <RCF/Config.hpp>
#include <RCF/SubscriptionService.hpp>

#if RCF_FEATURE_LEGACY==0

namespace RCF {

    boost::int32_t SubscriptionService::doRequestSubscription_Legacy(
        ClientStub &                    clientStubOrig, 
        const std::string &             publisherName,
        boost::uint32_t                 subToPubPingIntervalMs, 
        boost::uint32_t &               pubToSubPingIntervalMs,
        bool &                          pingsEnabled)
    {
        RCF_UNUSED_VARIABLE(clientStubOrig);
        RCF_UNUSED_VARIABLE(publisherName);
        RCF_UNUSED_VARIABLE(subToPubPingIntervalMs);
        RCF_UNUSED_VARIABLE(pubToSubPingIntervalMs);
        RCF_UNUSED_VARIABLE(pingsEnabled);

        RCF_THROW( Exception( 
            _RcfError_NotSupportedInThisBuild("Legacy subscription request") ) );

        return RcfError_NotSupportedInThisBuild;
    }

    void SubscriptionService::doRequestSubscriptionAsync_Legacy(
        ClientStub &                    clientStubOrig, 
        const std::string &             publisherName,
        boost::uint32_t                 outgoingPingIntervalMs, 
        RcfClientPtr                    rcfClientPtr,
        const SubscriptionParms &       parms)
    {
        RCF_UNUSED_VARIABLE(clientStubOrig);
        RCF_UNUSED_VARIABLE(publisherName);
        RCF_UNUSED_VARIABLE(outgoingPingIntervalMs);
        RCF_UNUSED_VARIABLE(rcfClientPtr);
        RCF_UNUSED_VARIABLE(parms);

        RCF_THROW( Exception( 
            _RcfError_NotSupportedInThisBuild("Legacy subscription request") ) );
    }

    void SubscriptionService::doRequestSubscriptionAsync_Legacy_Complete(
        ClientStubPtr                   clientStubPtr,
        Future<boost::int32_t>          fRet,
        const std::string &             publisherName,
        RcfClientPtr                    rcfClientPtr,
        OnSubscriptionDisconnect        onDisconnect,
        OnAsyncSubscribeCompleted       onCompletion,
        Future<boost::uint32_t>         pubToSubPingIntervalMs,
        bool                            pingsEnabled)
    {
        RCF_UNUSED_VARIABLE(clientStubPtr);
        RCF_UNUSED_VARIABLE(fRet);
        RCF_UNUSED_VARIABLE(publisherName);
        RCF_UNUSED_VARIABLE(rcfClientPtr);
        RCF_UNUSED_VARIABLE(onDisconnect);
        RCF_UNUSED_VARIABLE(onCompletion);
        RCF_UNUSED_VARIABLE(pubToSubPingIntervalMs);
        RCF_UNUSED_VARIABLE(pingsEnabled);

        RCF_THROW( Exception( 
            _RcfError_NotSupportedInThisBuild("Legacy subscription request") ) );
    }

} // namespace RCF

#else

#include <RCF/ServerInterfaces.hpp>

namespace RCF {

        boost::int32_t SubscriptionService::doRequestSubscription_Legacy(
        ClientStub &            clientStubOrig, 
        const std::string & publisherName,
        boost::uint32_t subToPubPingIntervalMs, 
        boost::uint32_t &       pubToSubPingIntervalMs,
        bool & pingsEnabled)
    {
        RcfClient<I_RequestSubscription> client(clientStubOrig);
        client.getClientStub().setTransport(clientStubOrig.releaseTransport());

        boost::int32_t ret = 0;
        if (clientStubOrig.getRuntimeVersion() < 8)
        {
            pingsEnabled = false;

            ret = client.RequestSubscription(
                Twoway, 
                publisherName);
        }
        else
        {
            pingsEnabled = true;

            ret = client.RequestSubscription(
                Twoway, 
                publisherName, 
                subToPubPingIntervalMs, 
                pubToSubPingIntervalMs);
        }

        clientStubOrig.setTransport( client.getClientStub().releaseTransport() );

        return ret;
    }

    void SubscriptionService::doRequestSubscriptionAsync_Legacy_Complete(
        ClientStubPtr                           clientStubPtr,
        Future<boost::int32_t>                  fRet,
        const std::string &                     publisherName,
        RcfClientPtr                            rcfClientPtr,
        OnSubscriptionDisconnect                onDisconnect,
        OnAsyncSubscribeCompleted               onCompletion,
        Future<boost::uint32_t>                 fPubToSubPingIntervalMs,
        bool                                    pingsEnabled)
    {
        boost::uint32_t ret = 0;
        boost::uint32_t pubToSubPingIntervalMs = 0;

        ExceptionPtr ePtr( clientStubPtr->getAsyncException().release() );
        if (!ePtr)
        {
            ret = *fRet;
            pubToSubPingIntervalMs = *fPubToSubPingIntervalMs;
        }

        createSubscriptionImplEnd(
            ePtr,
            clientStubPtr, 
            ret, 
            publisherName, 
            rcfClientPtr, 
            onDisconnect, 
            onCompletion, 
            pubToSubPingIntervalMs, 
            pingsEnabled);
    }

    void SubscriptionService::doRequestSubscriptionAsync_Legacy(
        ClientStub &            clientStubOrig, 
        const std::string &     publisherName,
        RcfClientPtr            rcfClientPtr,
        const SubscriptionParms & parms)
    {
        typedef RcfClient<I_RequestSubscription> AsyncClient;
        typedef boost::shared_ptr<AsyncClient> AsyncClientPtr;

        AsyncClientPtr asyncClientPtr( new AsyncClient(clientStubOrig) );

        asyncClientPtr->getClientStub().setTransport(
            clientStubOrig.releaseTransport());

        asyncClientPtr->getClientStub().setAsyncDispatcher(*mpServer);
      
        Future<boost::int32_t>      ret;
        Future<boost::uint32_t>     incomingPingIntervalMs;

        bool pingsEnabled = true;

        if (clientStubOrig.getRuntimeVersion() < 8)
        {
            pingsEnabled = false;

            ret = asyncClientPtr->RequestSubscription(

                AsyncTwoway( boost::bind( 
                    &SubscriptionService::doRequestSubscriptionAsync_Legacy_Complete, 
                    this,
                    asyncClientPtr->getClientStubPtr(),
                    ret,
                    publisherName,
                    rcfClientPtr,
                    parms.mOnDisconnect,
                    parms.mOnAsyncSubscribeCompleted,
                    incomingPingIntervalMs,
                    pingsEnabled)),

                publisherName);
        }
        else
        {
            boost::uint32_t outgoingPingIntervalMs = mPingIntervalMs;

            ret = asyncClientPtr->RequestSubscription(

                AsyncTwoway( boost::bind( 
                    &SubscriptionService::doRequestSubscriptionAsync_Legacy_Complete, 
                    this,
                    asyncClientPtr->getClientStubPtr(),
                    ret,
                    publisherName,
                    rcfClientPtr,
                    parms.mOnDisconnect,
                    parms.mOnAsyncSubscribeCompleted,
                    incomingPingIntervalMs,
                    pingsEnabled) ),

                publisherName,
                outgoingPingIntervalMs,
                incomingPingIntervalMs);
        }
    }

} // namespace RCF

#endif
