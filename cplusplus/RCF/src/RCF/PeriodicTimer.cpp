
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

#include <RCF/PeriodicTimer.hpp>

#include <RCF/AmiThreadPool.hpp>
#include <RCF/AsioDeadlineTimer.hpp>
#include <RCF/Service.hpp>

namespace RCF {

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 ) // warning C4355: 'this' : used in base member initializer list
#endif

    PeriodicTimer::PeriodicTimer(I_Service & service, boost::uint32_t intervalMs) :
        mTcbPtr( new TimerControlBlock(this) ),
        mService(service),
        mIntervalMs(intervalMs),
        mAsioTimerPtr()
    {
    }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    PeriodicTimer::~PeriodicTimer()
    {
        stop();
    }

    void PeriodicTimer::start()
    {
        mAsioTimerPtr.reset( new AsioTimer(getAmiThreadPool().getIoService()) );

        {
            Lock lock(mTcbPtr->mMutex);
            mTcbPtr->mpPeriodicTimer = this;
        }
        setTimer();
    }

    void PeriodicTimer::stop()
    {
        {
            Lock lock(mTcbPtr->mMutex);
            mTcbPtr->mpPeriodicTimer = NULL;
        }

        mAsioTimerPtr.reset();
    }

    void PeriodicTimer::setIntervalMs(boost::uint32_t intervalMs)
    {
        mIntervalMs = intervalMs;
    }

    boost::uint32_t PeriodicTimer::getIntervalMs()
    {
        return mIntervalMs;
    }

    class PeriodicTimerSentry
    {
    public:
        PeriodicTimerSentry(PeriodicTimer & periodicTimer) :
            mPeriodicTimer(periodicTimer)
        {
        }
        ~PeriodicTimerSentry()
        {
            mPeriodicTimer.setTimer();
        }
        PeriodicTimer & mPeriodicTimer;
    };

    void PeriodicTimer::setTimer()
    {
        if (mIntervalMs)
        {
            mAsioTimerPtr->mImpl.expires_from_now( boost::posix_time::milliseconds(mIntervalMs) );
            mAsioTimerPtr->mImpl.async_wait( boost::bind(&PeriodicTimer::sOnTimer, _1, mTcbPtr) );
        }
    }

    void PeriodicTimer::sOnTimer(
        const AsioErrorCode & ec, 
        TimerControlBlockPtr tcbPtr)
    {
        RCF_UNUSED_VARIABLE(ec);

        Lock lock(tcbPtr->mMutex);
        if (tcbPtr->mpPeriodicTimer)
        {
            PeriodicTimer & timer = * tcbPtr->mpPeriodicTimer;
            PeriodicTimerSentry sentry(timer);
            timer.mLastRunTimer.restart();
            timer.mService.onTimer();
        }
    }

} // namespace RCF
