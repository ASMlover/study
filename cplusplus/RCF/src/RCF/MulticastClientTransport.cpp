
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

#include <RCF/MulticastClientTransport.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Future.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/Timer.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    TransportType MulticastClientTransport::getTransportType()
    {
        return Tt_Unknown;
    }

    ClientTransportAutoPtr MulticastClientTransport::clone() const
    {
        RCF_ASSERT(0);
        return ClientTransportAutoPtr();
    }

    EndpointPtr MulticastClientTransport::getEndpointPtr() const
    {
        RCF_ASSERT(0);
        return EndpointPtr();
    }
    
    // Used to coordinate the async sends for a published message.
    class PublishCompletionInfo
    {
    public:

        PublishCompletionInfo(std::size_t notifyOnCount) : 
            mMutex(),
            mCondition(),
            mCompletedCount(0),
            mFailedCount(0),
            mNotifyOnCount(notifyOnCount)
        {
        }

        void notifyCompletion(bool completedOk)
        {
            Lock lock(mMutex);
            if (completedOk)
            {
                ++mCompletedCount;
            }
            else
            {
                ++mFailedCount;
            }

            RCF_ASSERT(mCompletedCount + mFailedCount <= mNotifyOnCount);
            if (mCompletedCount + mFailedCount == mNotifyOnCount)
            {
                mCondition.notify_all(lock);
            }
        }

        void wait(boost::uint32_t waitMs)
        {
            Timer waitTimer;

            Lock lock(mMutex);

            while (
                    !waitTimer.elapsed(waitMs)
                &&  mCompletedCount + mFailedCount < mNotifyOnCount)
            {
                boost::uint32_t timeUsedSoFarMs = waitTimer.getDurationMs();
                timeUsedSoFarMs = RCF_MIN(timeUsedSoFarMs, waitMs);
                boost::uint32_t timeRemainingMs = waitMs - timeUsedSoFarMs;
                bool timedOut = mCondition.timed_wait(lock, timeRemainingMs);
                if (    !timedOut 
                    &&  (mCompletedCount + mFailedCount == mNotifyOnCount))
                {
                    break;
                }
            }
        }

        std::size_t getCompletionCount()
        {
            Lock lock(mMutex);
            return mCompletedCount + mFailedCount;
        }

    private:

        Mutex           mMutex;
        Condition       mCondition;

        std::size_t     mCompletedCount;
        std::size_t     mFailedCount;
        std::size_t     mNotifyOnCount;

    };

    class PublishCompletionHandler : public ClientTransportCallback
    {
    public:

        PublishCompletionHandler() :
            mpClientTransport(NULL),
            mpInfo(NULL),
            mCompleted(false),
            mOk(false),
            mError()
        {
        }

        PublishCompletionHandler(
            ClientTransport * pClientTransport,
            PublishCompletionInfo * pInfo) : 
                mpClientTransport(pClientTransport),
                mpInfo(pInfo),
                mCompleted(false),
                mOk(false),
                mError()
        {
        }

        void onConnectCompleted(bool alreadyConnected = false)
        {
            RCF_UNUSED_VARIABLE(alreadyConnected);
            RCF_ASSERT(0);
        }

        void onSendCompleted()
        {
            mCompleted = true;
            mOk = true;
            mError = "";
            mpInfo->notifyCompletion(mOk);
        }

        void onReceiveCompleted()
        {
            RCF_ASSERT(0);
        }

        void onTimerExpired()
        {
            RCF_ASSERT(0);
        }

        void onError(const std::exception &e)
        {
            mCompleted = true;
            mOk = false;
            mError = e.what();
            mpInfo->notifyCompletion(mOk);
        }

        ClientTransport *       mpClientTransport;
        PublishCompletionInfo * mpInfo;

        bool                    mCompleted;
        bool                    mOk;
        std::string             mError;
    };


    int MulticastClientTransport::send(
        ClientTransportCallback &           clientStub,
        const std::vector<ByteBuffer> &     data,
        unsigned int                        timeoutMs)
    {
        // NB: As the same buffer is sent on all transports, the transports and
        // filters should never modify the buffer. Any transport that transforms
        // data needs to do so in a separate per-transport buffer.

        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_LOG_2()(lengthByteBuffers(data))(timeoutMs) 
            << "MulticastClientTransport::send() - entry.";

        mLastRequestSize = lengthByteBuffers(data);
        mRunningTotalBytesSent += mLastRequestSize;

        bringInNewTransports();

        Lock lock(mClientTransportsMutex);

        std::size_t transportsInitial = mClientTransports.size();

        PublishCompletionInfo info( mClientTransports.size() );

        // Setup completion handlers.
        std::vector<PublishCompletionHandler> handlers( mClientTransports.size() );
        for (std::size_t i=0; i<mClientTransports.size(); ++i)
        {
            ClientTransport * pTransport = (*mClientTransports[i]).get();
            handlers[i] = PublishCompletionHandler(pTransport, &info);
        }

        // Async send on all transports.
        for (std::size_t i=0; i<handlers.size(); ++i)
        {
            try
            {

                handlers[i].mpClientTransport->setAsync(true);
                handlers[i].mpClientTransport->send(handlers[i], data, 0);
            }
            catch(const Exception &e)
            {
                Exception err( _RcfError_SyncPublishError(e.what()) );
                handlers[i].onError(err);
            }
        }

        // Wait for async completions.
        boost::uint32_t completionDurationMs = 0;
        {
            Timer timer;
            info.wait(timeoutMs);
            completionDurationMs = timer.getDurationMs();
        }

        // Cancel any outstanding sends.
        for (std::size_t i=0; i<handlers.size(); ++i)
        {
            if (!handlers[i].mCompleted)
            {
                (*mClientTransports[i])->cancel();

                RCF_LOG_2()(i)
                    << "MulticastClientTransport::send() - cancel send.";
            }
        }

        // Wait for canceled ops to complete.
        boost::uint32_t cancelDurationMs = 0;
        {
            Timer timer;
            info.wait(timeoutMs);
            cancelDurationMs = timer.getDurationMs();
        }

        RCF_ASSERT(info.getCompletionCount() == handlers.size());

        // Close and remove any subscriber transports with errors.
        std::size_t transportsRemoved = 0;
        for (std::size_t i=0; i<handlers.size(); ++i)
        {
            RCF_ASSERT(handlers[i].mCompleted);
            if (!handlers[i].mOk)
            {
                mClientTransports[i] = ClientTransportAutoPtrPtr();
                ++transportsRemoved;

                RCF_LOG_2()(i)(handlers[i].mCompleted)(handlers[i].mOk)(handlers[i].mError) 
                    << "MulticastClientTransport::send() - remove subscriber transport.";
            }
        }
        eraseRemove(mClientTransports, ClientTransportAutoPtrPtr());

        clientStub.onSendCompleted();

        std::size_t transportsFinal = transportsInitial - transportsRemoved;

        RCF_LOG_2()
            (lengthByteBuffers(data))(completionDurationMs)(cancelDurationMs)(transportsInitial)(transportsFinal)
            << "MulticastClientTransport::send() - exit.";

        return 1;
    }

    int MulticastClientTransport::receive(
        ClientTransportCallback &clientStub,
        ByteBuffer &byteBuffer,
        unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(clientStub);
        RCF_UNUSED_VARIABLE(byteBuffer);
        RCF_UNUSED_VARIABLE(timeoutMs);
        RCF_ASSERT(0);
        return 1;
    }

    bool MulticastClientTransport::isConnected()
    {
        return true;
    }

    void MulticastClientTransport::connect(ClientTransportCallback &clientStub, unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(clientStub);
        RCF_UNUSED_VARIABLE(timeoutMs);
        clientStub.onConnectCompleted(true);
    }

    void MulticastClientTransport::disconnect(unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
    }

    void MulticastClientTransport::addTransport(
        ClientTransportAutoPtr clientTransportAutoPtr)
    {
        Lock lock(mAddedClientTransportsMutex);

        clientTransportAutoPtr->setAsync(false);

        mAddedClientTransports.push_back( ClientTransportAutoPtrPtr( 
            new ClientTransportAutoPtr(clientTransportAutoPtr) ) );
    }

    void MulticastClientTransport::bringInNewTransports()
    {
        ClientTransportList addedClientTransports;

        {
            Lock lock(mAddedClientTransportsMutex);
            addedClientTransports.swap(mAddedClientTransports);
        }

        Lock lock(mClientTransportsMutex);

        std::copy(
            addedClientTransports.begin(),
            addedClientTransports.end(),
            std::back_inserter(mClientTransports));
    }

    void MulticastClientTransport::setTransportFilters(
        const std::vector<FilterPtr> &)
    {
        // not supported
    }

    void MulticastClientTransport::getTransportFilters(
        std::vector<FilterPtr> &)
    {
        // not supported
    }

    void MulticastClientTransport::setTimer(
        boost::uint32_t timeoutMs,
        ClientTransportCallback *pClientStub)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
        RCF_UNUSED_VARIABLE(pClientStub);
    }

    void MulticastClientTransport::dropIdleTransports()
    {
        bringInNewTransports();

        Lock lock(mClientTransportsMutex);

        bool needToRemove = false;

        ClientTransportList::iterator iter;
        for (iter = mClientTransports.begin(); iter != mClientTransports.end(); ++iter)
        {
            RCF::ClientTransport & transport = ***iter;
            RcfSessionWeakPtr rcfSessionWeakPtr = transport.getRcfSession();
            if ( rcfSessionWeakPtr == RcfSessionWeakPtr() )
            {
                // HTTP/HTTPS connections do not hold on to the RcfSession and can't receive pings.
                continue;
            }
            RcfSessionPtr rcfSessionPtr = rcfSessionWeakPtr.lock();
            if (!rcfSessionPtr)
            {
                RCF_LOG_2() << "Dropping subscription. Subscriber has closed connection.";
                iter->reset();
                needToRemove = true;
            }
            else
            {
                boost::uint32_t pingIntervalMs = rcfSessionPtr->getPingIntervalMs();
                if (pingIntervalMs)
                {
                    RCF::Timer pingTimer( rcfSessionPtr->getPingTimestamp() );
                    if (pingTimer.elapsed(5000 + 2*pingIntervalMs))
                    {
                        std::string subscriberUrl = rcfSessionPtr->getClientAddress().string();
                        
                        RCF_LOG_2()(subscriberUrl)(pingIntervalMs) 
                            << "Dropping subscription. Subscriber has not sent pings within the expected ping interval.";

                        iter->reset();
                        needToRemove = true;
                    }
                }
            }
        }

        if (needToRemove)
        {
            eraseRemove(mClientTransports, ClientTransportAutoPtrPtr());
        }
    }

    void MulticastClientTransport::pingAllTransports()
    {
        bringInNewTransports();

        Lock lock(mClientTransportsMutex);

        if (!mMulticastTemp.get())
        {
            mMulticastTemp.reset( new MulticastClientTransport() );
        }

        MulticastClientTransport & multicastTemp = 
            static_cast<MulticastClientTransport &>(*mMulticastTemp);

        multicastTemp.mClientTransports.resize(0);

        ClientTransportList::iterator iter;
        for (iter = mClientTransports.begin(); iter != mClientTransports.end(); ++iter)
        {
            ClientTransport & transport = ***iter;
            RcfSessionPtr rcfSessionPtr = transport.getRcfSession().lock();
            if (rcfSessionPtr)
            {
                boost::uint32_t pingIntervalMs = rcfSessionPtr->getPingIntervalMs();
                if (pingIntervalMs)
                {
                    multicastTemp.mClientTransports.push_back(*iter);
                }
            }
        }

        I_RcfClient nullClient("", mMulticastTemp );
        nullClient.getClientStub().ping(RCF::Oneway);
        mMulticastTemp.reset( nullClient.getClientStub().releaseTransport().release() );
        multicastTemp.mClientTransports.resize(0);
    }

    void MulticastClientTransport::close()
    {
        Lock lock(mClientTransportsMutex);
        mClientTransports.clear();
    }

    std::size_t MulticastClientTransport::getTransportCount()
    {
        Lock lock(mClientTransportsMutex);
        return mClientTransports.size();
    }

} // namespace RCF
