
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

#include <RCF/PingBackService.hpp>

#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    PingBackService::PingBackService() : 
        mEnabled(true),
        mLazyStarted(false)
    {
    }

    void PingBackService::onServiceAdded(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);

        mTaskEntries.clear();

        mTaskEntries.push_back( TaskEntry(
            boost::bind(&PingBackService::cycle, this, _1),
            boost::bind(&PingBackService::stop, this),
            "RCF Pingback",
            false));
    }

    void PingBackService::onServiceRemoved(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }

    void PingBackService::stop()
    {
    }

    void PingBackService::cycle(int timeoutMs)
    {
        RCF::ThreadInfoPtr tiPtr = getTlsThreadInfoPtr();
        RCF::ThreadPool & threadPool = tiPtr->getThreadPool();

        mTimerHeap.rebase();

        PingBackTimerEntry entry;

        while (     !threadPool.shouldStop()
                &&  mTimerHeap.getExpiredEntry(entry))
        {
            // Is the session still alive?
            RcfSessionPtr rcfSessionPtr( entry.second.lock() );
            if (rcfSessionPtr)
            {
                Lock lock(rcfSessionPtr->mIoStateMutex);

                // Is the timer entry still there? It can't change while we hold mIoStateMutex.
                if (mTimerHeap.compareTop(entry))
                {
                    // Calculate the next ping back time.
                    boost::uint32_t pingBackIntervalMs = rcfSessionPtr->getPingBackIntervalMs();
                    pingBackIntervalMs = RCF_MAX(pingBackIntervalMs, boost::uint32_t(1000));

                    boost::uint32_t nextFireMs = 
                        RCF::getCurrentTimeMs() + pingBackIntervalMs;

                    PingBackTimerEntry nextEntry(nextFireMs, rcfSessionPtr);

                    mTimerHeap.remove(entry);
                    mTimerHeap.add(nextEntry);
                    rcfSessionPtr->mPingBackTimerEntry = nextEntry;

                    // Only send a pingback, if previous one has completed.
                    if (!rcfSessionPtr->mWritingPingBack)
                    {
                        rcfSessionPtr->sendPingBack();
                    }

                    mTimerHeap.add(nextEntry);
                }
            }
        } 

        boost::uint32_t queueTimeoutMs = RCF_MIN(
            static_cast<boost::uint32_t>(timeoutMs),
            mTimerHeap.getNextEntryTimeoutMs());

        if (!threadPool.shouldStop())
        {
            Lock lock(mMutex);
            mCondition.timed_wait(lock, queueTimeoutMs);
        }            
    }

    PingBackService::Entry PingBackService::registerSession(RcfSessionPtr rcfSessionPtr)
    {
        boost::uint32_t pingBackIntervalMs = rcfSessionPtr->getPingBackIntervalMs();

        RCF_ASSERT( pingBackIntervalMs );

        if (pingBackIntervalMs < 1000)
        {
            RCF_THROW( Exception(_RcfError_PingBackInterval(pingBackIntervalMs, 1000) ) );
        }

        {
            Lock lock(mMutex);
            if (!mLazyStarted)
            {
                mTaskEntries[0].start();
                mLazyStarted = true;
            }
        }

        // We don't send a pingback right away, because it would interfere with
        // remote calls that complete quickly. Inserting a ping back in the response
        // stream will fragment it and ruin network performance.

        // Schedule next pingback.
        Lock lock(mMutex);
        boost::uint32_t nextFireMs = RCF::getCurrentTimeMs() + pingBackIntervalMs;
        Entry entry(nextFireMs, rcfSessionPtr);
        mTimerHeap.add(entry);
        mCondition.notify_all(lock);
        return entry;
    }

    void PingBackService::unregisterSession(const Entry & entry)
    {
        mTimerHeap.remove(entry);
    }

}
