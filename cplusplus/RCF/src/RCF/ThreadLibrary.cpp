
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

#include <RCF/ThreadLibrary.hpp>

#include <RCF/Config.hpp>
#include <RCF/Export.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    Mutex & getRootMutex()
    {
        static Mutex * gpRootMutex = new Mutex;
        return *gpRootMutex;
    }

#ifdef BOOST_WINDOWS

    RecursiveMutex::RecursiveMutex() : RCF::detail::win_mutex(true)
    {
    }

    RecursiveMutex::~RecursiveMutex()
    {
    }

#else

    RecursiveMutex::RecursiveMutex() : 
        mIsLocked(false), 
        mOwner(0), 
        mLockCount(0)
    {
    }

    RecursiveMutex::~RecursiveMutex()
    {
    }    

    void RecursiveMutex::lock()
    {
        Lock lock(mMutex);
        if (mIsLocked && mOwner == getCurrentThreadId())
        {
            ++mLockCount;
            return;
        }

        while (mIsLocked)
        {
            mCondition.wait(lock);
        }

        mIsLocked = true;
        ++mLockCount;
        mOwner = getCurrentThreadId();
    }

    void RecursiveMutex::unlock()
    {
        Lock lock(mMutex);
        --mLockCount;
        if (mLockCount == 0)
        {
            mIsLocked = false;
            mCondition.notify_all(lock);
        }
    }

#endif

#ifdef BOOST_WINDOWS

    void sleepMs(boost::uint32_t msec)
    {
        ::Sleep(msec);
    }

#else

    void sleepMs(boost::uint32_t msec)
    {
        Mutex mx;
        Lock lock(mx);
        Condition cond;
        bool timedOut = !cond.timed_wait(lock, msec);
        RCF_UNUSED_VARIABLE(timedOut);
        RCF_ASSERT(timedOut);
    }

#endif

}
