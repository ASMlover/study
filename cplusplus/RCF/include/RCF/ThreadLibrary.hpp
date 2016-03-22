
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

#ifndef INCLUDE_RCF_THREADLIBRARY_HPP
#define INCLUDE_RCF_THREADLIBRARY_HPP

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Config.hpp>
#include <RCF/Export.hpp>

#include <RCF/AsioFwd.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>

#ifdef BOOST_WINDOWS
#include <windows.h>
#endif

namespace RCF {
    namespace detail {
        typedef boost::noncopyable noncopyable;
    }
}

#include <RCF/thread/event.hpp>
#include <RCF/thread/mutex.hpp>
#include <RCF/thread/thread.hpp>

#ifdef RCF_USE_BOOST_TLS
#include <boost/thread/tss.hpp>
#else
#include <RCF/thread/tss_ptr.hpp>
#endif

namespace RCF {

    // Multi-threading primitives, based on the ones in asio.

    typedef RCF::detail::thread                 Thread;
    typedef RCF::detail::mutex                  Mutex;
    typedef RCF::detail::mutex::scoped_lock     Lock;
    typedef RCF::detail::event                  Condition;

#ifdef BOOST_WINDOWS
    typedef int                                 ThreadId;
#else
    typedef pthread_t                           ThreadId;
#endif
    
#ifdef RCF_USE_BOOST_TLS

    template<typename T>
    class ThreadSpecificPtr : public boost::thread_specific_ptr<T>
    {
    };

#else

    template<typename T>
    class ThreadSpecificPtr : public RCF::detail::tss_ptr<T>
    {
    public:
        void reset(T * pt = NULL)
        {
            RCF::detail::tss_ptr<T>::operator =(pt);
        }

        T * get() const
        {
            return RCF::detail::tss_ptr<T>::operator T*();
        }

        T * operator->() const
        {
            return RCF::detail::tss_ptr<T>::operator T*();
        }

        typedef ThreadSpecificPtr Val;
    };

#endif

    // Simple read-write mutex.

    class ReadWriteMutex;

    class RCF_EXPORT ReadLock : boost::noncopyable
    {
    public:
        ReadLock(ReadWriteMutex &rwm);
        ~ReadLock();
        void lock();
        void unlock();

    private:
        ReadWriteMutex &                    rwm;
        bool                                locked;
    };

    class RCF_EXPORT WriteLock : boost::noncopyable
    {
    public:
        WriteLock(ReadWriteMutex &rwm);
        ~WriteLock();
        void lock();
        void unlock();

    private:
        ReadWriteMutex &                    rwm;
        Lock                                readLock;
        Lock                                writeLock;
        bool                                locked;
    };

    enum ReadWritePriority
    {
        ReaderPriority,
        WriterPriority
    };

    class RCF_EXPORT ReadWriteMutex : boost::noncopyable
    {
    public:
        ReadWriteMutex(ReadWritePriority rwsp);

    private:

        void waitOnReadUnlock(Lock &lock);
        void notifyReadUnlock(Lock &lock);

        Mutex                                   readMutex;
        Mutex                                   writeMutex;
        Condition                               readUnlockEvent;
        int                                     readerCount;

    public:

        friend class ReadLock;
        friend class WriteLock;

    };

#ifdef BOOST_WINDOWS
    
    // On Windows critical sections are automatically recursive. All
    // this class does is disable the recursive locking assert in win_mutex.
    class RCF_EXPORT RecursiveMutex : public RCF::detail::win_mutex
    {
    public:
        RecursiveMutex();
        ~RecursiveMutex();
    };

    typedef Lock                                RecursiveLock;

#else

    // Some pthreads versions have built in recursive mutexes 
    // (PTHREAD_MUTEX_RECURSIVE). For portability we're using
    // a custom implemenation instead.

    class RCF_EXPORT RecursiveMutex : boost::noncopyable
    {
    public:
        RecursiveMutex();
        ~RecursiveMutex();

    private:

        friend class RCF::detail::scoped_lock<RecursiveMutex>;

        void lock();
        void unlock();

        Mutex           mMutex;
        Condition       mCondition;
        bool            mIsLocked;
        ThreadId        mOwner;
        std::size_t     mLockCount;

    };

    typedef RCF::detail::scoped_lock<RecursiveMutex> RecursiveLock;

#endif

    typedef boost::shared_ptr<RecursiveLock>    RecursiveLockPtr;
    typedef boost::shared_ptr<RecursiveMutex>   RecursiveMutexPtr;

    typedef boost::shared_ptr<Thread>           ThreadPtr;
    typedef boost::shared_ptr<ReadWriteMutex>   ReadWriteMutexPtr;
    typedef boost::shared_ptr<Mutex>            MutexPtr;
    typedef boost::shared_ptr<Lock>             LockPtr;
    typedef boost::shared_ptr<Condition>        ConditionPtr;


    RCF_EXPORT ThreadId getCurrentThreadId();

    // Time in ms since ca 1970, modulo 65536 s (turns over every ~18.2 hrs).
    RCF_EXPORT boost::uint32_t getCurrentTimeMs();

    // Generate a timeout value for the given ending time.
    // Returns zero if endTime <= current time <= endTime+10%of timer resolution, otherwise returns a nonzero duration in ms.
    // Timer resolution as above (18.2 hrs).
    static const unsigned int MaxTimeoutMs = (((unsigned int)-1)/10)*9;
    RCF_EXPORT boost::uint32_t generateTimeoutMs(unsigned int endTimeMs);

    RCF_EXPORT Mutex & getRootMutex();

    RCF_EXPORT void sleepMs(boost::uint32_t msec);
}

#endif // ! INCLUDE_RCF_THREADLIBRARY_HPP
