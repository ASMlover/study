
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

#ifndef INCLUDE_RCF_PUBLISHINGSERVICE_HPP
#define INCLUDE_RCF_PUBLISHINGSERVICE_HPP

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/PeriodicTimer.hpp>
#include <RCF/Service.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Timer.hpp>

namespace RCF {

    class                                           RcfServer;
    class                                           I_RcfClient;
    class                                           RcfSession;
    class                                           ClientTransport;
    typedef boost::shared_ptr<I_RcfClient>          RcfClientPtr;
    typedef boost::shared_ptr<ClientTransport>    ClientTransportPtr;

    class                                           PublisherBase;
    typedef boost::shared_ptr<PublisherBase>        PublisherPtr;
    typedef boost::weak_ptr<PublisherBase>          PublisherWeakPtr;

    class                                           PublishingService;
    
    typedef boost::function2<bool, RcfSession &, const std::string &> OnSubscriberConnect;
    typedef boost::function2<void, RcfSession &, const std::string &> OnSubscriberDisconnect;

    class RCF_EXPORT PublisherParms
    {
    public:

        void setTopicName(const std::string & topicName);
        std::string getTopicName() const;
        void setOnSubscriberConnect(OnSubscriberConnect onSubscriberConnect);
        void setOnSubscriberDisconnect(OnSubscriberDisconnect onSubscriberDisconnect);

    private:

        friend class PublishingService;

        std::string             mTopicName;
        OnSubscriberConnect     mOnSubscriberConnect;
        OnSubscriberDisconnect  mOnSubscriberDisconnect;
    };


    class RCF_EXPORT PublisherBase : boost::noncopyable
    {
    public:
        PublisherBase(PublishingService & pubService, const PublisherParms & parms);
        ~PublisherBase();

        std::string     getTopicName();
        std::size_t     getSubscriberCount();
        void            close();

    protected:

        friend class PublishingService;

        void init();

        PublishingService &     mPublishingService;
        PublisherParms          mParms;
        bool                    mClosed;
        std::string             mTopicName;
        RcfClientPtr            mRcfClientPtr;
    };

    // Rename to Topic?
    template<typename Interface>
    class Publisher : public PublisherBase
    {
    public:

        typedef typename Interface::RcfClientT RcfClientT;

        Publisher(PublishingService & pubService, const PublisherParms & parms) :
            PublisherBase(pubService, parms),
            mpClient(NULL)
        {
            if (mTopicName.empty())
            {
                mTopicName = getInterfaceName<Interface>();
            }

            mRcfClientPtr.reset( new RcfClientT( ClientStub(mTopicName) ) );
            mpClient = dynamic_cast<RcfClientT *>(mRcfClientPtr.get());

            init();
        }

        RcfClientT & publish()
        {
            RCF_ASSERT(!mClosed);
            return *mpClient;
        }

    private:

        RcfClientT * mpClient;
    };   

    class I_RequestSubscription;
    template<typename T> class RcfClient; 

    class RCF_EXPORT PublishingService :
        public I_Service,
        boost::noncopyable
    {
    public:
        PublishingService();

        ~PublishingService();

        template<typename Interface>
        boost::shared_ptr< Publisher<Interface> > createPublisher(
            const PublisherParms & parms)
        {
            boost::shared_ptr< Publisher<Interface> > publisherPtr(
                new Publisher<Interface>(*this, parms) );

            std::string topicName = publisherPtr->getTopicName();

            RCF_ASSERT(topicName.size() > 0);
            Lock lock(mPublishersMutex);
            mPublishers[topicName] = publisherPtr;
            return publisherPtr;
        }

        void setPingIntervalMs(boost::uint32_t pingIntervalMs);
        boost::uint32_t getPingIntervalMs() const;


    private:

        friend class RcfClient<I_RequestSubscription>;
        friend class PublisherBase;
        friend class PublishingServicePb;
        friend class RcfSession;

        boost::int32_t  RequestSubscription(
                            const std::string &subscriptionName);

        boost::int32_t  RequestSubscription(
                            const std::string &subscriptionName,
                            boost::uint32_t subToPubPingIntervalMs,
                            boost::uint32_t & pubToSubPingIntervalMs);

    private:

        void            onServiceAdded(RcfServer &server);
        void            onServiceRemoved(RcfServer &server);
        void            onServerStart(RcfServer &server);
        void            onServerStop(RcfServer &server);

        void            addSubscriberTransport(
                            RcfSession &session,
                            const std::string &publisherName,
                            ClientTransportAutoPtrPtr clientTransportAutoPtrPtr);

        void            closePublisher(const std::string & name);

        // TODO: should be weak_ptr here probably. ~Publisher() will need
        // to clean them up ?
        typedef std::map<std::string, PublisherWeakPtr>     Publishers;

        Mutex                           mPublishersMutex;
        Publishers                      mPublishers;

        boost::uint32_t                 mPingIntervalMs;
        PeriodicTimer                   mPeriodicTimer;

        virtual void onTimer();
        void pingAllSubscriptions();
        void harvestExpiredSubscriptions();

    };

    typedef boost::shared_ptr<PublishingService> PublishingServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_PUBLISHINGSERVICE_HPP
