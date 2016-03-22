
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

#include <boost/config.hpp>
#include <boost/cstdint.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/util/UnusedVariable.hpp>

#ifdef BOOST_HAS_PTHREADS
#include <pthread.h>
#endif

#ifdef NDEBUG
// To test timer roll over, we can set the tick counter to start at a negative value.
//static const boost::uint32_t OffsetMs = 15*1000;
static const boost::uint32_t OffsetMs = 0;
#else
static const boost::uint32_t OffsetMs = 0;
#endif

#ifdef BOOST_WINDOWS

// Windows implementation, using GetTickCount().
#include <Windows.h>

namespace RCF {

    boost::uint32_t getCurrentTimeMs()
    {
        static boost::uint32_t BaseTickCountMs = GetTickCount();
        boost::uint32_t timeMs = GetTickCount();
        return  timeMs - BaseTickCountMs - OffsetMs;
    }

    ThreadId getCurrentThreadId() 
    { 
        return GetCurrentThreadId(); 
    }

} // namespace RCF

#else

// Non-Windows implementation, using gettimeofday().
#include <sys/time.h>

namespace RCF {

    // TODO: any issues with monotonicity of gettimeofday()?
    boost::uint32_t getCurrentTimeMs()
    {
        static struct timeval start = {0};
        static bool init = false;
        if (!init)
        {
            init = true;
            gettimeofday(&start, NULL);
        }

        struct timeval now;
        gettimeofday(&now, NULL);

        long seconds =  now.tv_sec - start.tv_sec;
        long microseconds = now.tv_usec - start.tv_usec;
        boost::uint64_t timeMs = boost::uint64_t(seconds)*1000 + microseconds/1000;
        timeMs = timeMs & 0xFFFFFFFF;
        return static_cast<boost::uint32_t>(timeMs) - OffsetMs;
    }

    ThreadId getCurrentThreadId() 
    { 
        return pthread_self(); 
    }

} // namespace RCF

#endif

// ReadWriteMutex implementation
namespace RCF {

    ReadWriteMutex::ReadWriteMutex(ReadWritePriority rwsp) :
        readerCount()
    {
        RCF_UNUSED_VARIABLE(rwsp);
    }

    void ReadWriteMutex::waitOnReadUnlock(Lock &lock)
    {
        readUnlockEvent.wait(lock);
    }

    void ReadWriteMutex::notifyReadUnlock(Lock &lock)
    {
        readUnlockEvent.notify_all(lock);
    }

    ReadLock::ReadLock(ReadWriteMutex &rwm) :
        rwm(rwm),
        locked()
    {
        lock();
    }

    ReadLock::~ReadLock()
    {
        unlock();
    }

    void ReadLock::lock()
    {
        if (!locked)
        {
            {
                Lock lock( rwm.readMutex );
                ++rwm.readerCount;
            }
            locked = true;
        }
    }

    void ReadLock::unlock()
    {
        if (locked)
        {
            {
                Lock lock( rwm.readMutex );
                --rwm.readerCount;
                rwm.notifyReadUnlock(lock);
            }
            locked = false;
        }
    }

    WriteLock::WriteLock(ReadWriteMutex &rwm) :
        rwm(rwm),
        readLock(rwm.readMutex, false),
        writeLock(rwm.writeMutex, false),
        locked()
    {
        lock();
    }

    WriteLock::~WriteLock()
    {
        unlock();
    }

    void WriteLock::lock()
    {
        if (!locked)
        {
            readLock.lock();
            while (rwm.readerCount > 0)
            {
                rwm.waitOnReadUnlock(readLock);
            }
            writeLock.lock();
            locked = true;
        }
    }

    void WriteLock::unlock()
    {
        if (locked)
        {
            writeLock.unlock();
            readLock.unlock();
            locked = false;
        }
    }

} // namespace RCF
