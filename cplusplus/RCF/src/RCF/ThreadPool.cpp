
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

#include <RCF/ThreadPool.hpp>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLocalData.hpp>

#include <RCF/Asio.hpp>
#include <RCF/AsioHandlerCache.hpp>
#include <RCF/AsioServerTransport.hpp>

// Setting thread names for debuggers etc.

#if defined(BOOST_WINDOWS) && !defined(__MINGW32__)

// Windows
namespace RCF {

    // The magic code to set thread names comes from MSDN: http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx

    const DWORD MS_VC_EXCEPTION = 0x406D1388;

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // must be 0x1000
        LPCSTR szName; // pointer to name (in user addr space)
        DWORD dwThreadID; // thread ID (-1=caller thread)
        DWORD dwFlags; // reserved for future use, must be zero
    } THREADNAME_INFO;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 6312) // warning C6312: Possible infinite loop:  use of the constant EXCEPTION_CONTINUE_EXECUTION in the exception-filter expression of a try-except.  Execution restarts in the protected block.
#pragma warning(disable: 6322) // warning C6322: Empty _except block.
#endif

    // 32 character limit on szThreadName apparently, or it gets truncated.
    void setWin32ThreadName(const std::string & threadName)
    {
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = threadName.c_str();
        info.dwThreadID = DWORD(-1);
        info.dwFlags = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except(EXCEPTION_CONTINUE_EXECUTION)
        {
        }
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace RCF

#elif defined(__MACH__) && defined(__APPLE__)

// OSX
#include <pthread.h>

namespace RCF {
    void setWin32ThreadName(const std::string & threadName)
    {
        pthread_setname_np(threadName.c_str());
    }
} // namespace RCF

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

// BSD
#include <pthread_np.h>

namespace RCF {
    void setWin32ThreadName(const std::string & threadName)
    {
        pthread_set_name_np(pthread_self(), threadName.c_str());
    }
} // namespace RCF

// Solaris
#elif defined(__SVR4) && defined(__sun)

namespace RCF {
    void setWin32ThreadName(const std::string & threadName)
    {
        // TODO
    }
} // namespace RCF

#elif defined(__linux__)

// Linux
#include <sys/prctl.h>

namespace RCF {
    void setWin32ThreadName(const std::string & threadName)
    {
        //pthread_setname_np(pthread_self(), threadName.c_str());
#ifdef PR_SET_NAME
        prctl(PR_SET_NAME, (unsigned long) threadName.c_str(), 0, 0, 0);
#endif

    }
} // namespace RCF

#else

// No-op
namespace RCF {
    void setWin32ThreadName(const std::string & threadName)
    {
    }
} // namespace RCF

#endif

namespace RCF {

    class AsioMuxer;
    typedef boost::shared_ptr<AsioMuxer> AsioMuxerPtr;
    typedef boost::weak_ptr<AsioMuxer> AsioMuxerWeakPtr;

    class TpTimeoutHandler
    {
    public:
        TpTimeoutHandler(AsioMuxerWeakPtr asioMuxerWeakPtr);
        void operator()(AsioErrorCode ec);
        AsioMuxerWeakPtr mAsioMuxerWeakPtr;
    };

    class TpDummyHandler
    {
    public:
        void operator()() {}
    };

    // Custom handler allocation, to avoid heap allocation.
    void * asio_handler_allocate(std::size_t size, TpTimeoutHandler * pHandler);
    void asio_handler_deallocate(void * pointer, std::size_t size, TpTimeoutHandler * pHandler);
    void * asio_handler_allocate(std::size_t size, TpDummyHandler * pHandler);
    void asio_handler_deallocate(void * pointer, std::size_t size, TpDummyHandler * pHandler);

    class AsioMuxer : public boost::enable_shared_from_this<AsioMuxer>
    {
    public:
        AsioMuxer() : 
            mIoService(), 
            mCycleTimer(mIoService)
        {
            mIoService.reset();
        }

        ~AsioMuxer()
        {
            mCycleTimer.mImpl.cancel();

            // Run any leftover handlers.
            while (mIoService.poll() > 0);
        }

        void startTimer()
        {
            mCycleTimer.mImpl.expires_from_now(
                boost::posix_time::milliseconds(10*1000));

            AsioMuxerWeakPtr thisWeakPtr = shared_from_this();
            mCycleTimer.mImpl.async_wait( TpTimeoutHandler(thisWeakPtr) );
        }

        void cycle(int timeoutMs)
        {
            RCF_ASSERT_GTEQ(timeoutMs , -1);
            RCF_UNUSED_VARIABLE(timeoutMs);

            mIoService.run_one();
        }

        void stopCycle()
        {
            mIoService.stop();
        }

        static void onTimer(
            AsioMuxerWeakPtr thisWeakPtr, 
            const AsioErrorCode & error)
        {
            AsioMuxerPtr thisPtr = thisWeakPtr.lock();

            // thisPtr will be NULL if this handler is called from ~AsioMuxer().
            if (!error && thisPtr)
            {
                ThreadInfoPtr threadInfoPtr = getTlsThreadInfoPtr();
                if (threadInfoPtr)
                {
                    ThreadPool & threadPool = threadInfoPtr->getThreadPool();
                    std::size_t threadCount = threadPool.getThreadCount();
                    RCF_ASSERT(threadCount >= 1);
                    for (std::size_t i=0; i<threadCount-1; ++i)
                    {
                        thisPtr->mIoService.post( TpDummyHandler() );
                    }
                }

                thisPtr->mCycleTimer.mImpl.expires_from_now(
                    boost::posix_time::milliseconds(10*1000));

                thisPtr->mCycleTimer.mImpl.async_wait(TpTimeoutHandler(thisWeakPtr) );
            }
        }

        AsioIoService mIoService;
        AsioTimer mCycleTimer;
    };


    TpTimeoutHandler::TpTimeoutHandler(AsioMuxerWeakPtr asioMuxerWeakPtr) : 
        mAsioMuxerWeakPtr(asioMuxerWeakPtr)
    {
    }

    void TpTimeoutHandler::operator()(AsioErrorCode ec)
    {
        AsioMuxer::onTimer(mAsioMuxerWeakPtr, ec);
    }

    AsioHandlerCache * gpTpTimeoutHandlerCache = NULL;
    AsioHandlerCache * gpTpDummyHandlerCache = NULL;

    void initTpHandlerCache()
    {
        gpTpTimeoutHandlerCache = new AsioHandlerCache(); 
        gpTpDummyHandlerCache = new AsioHandlerCache();
    }

    void deinitTpHandlerCache()
    {
        delete gpTpTimeoutHandlerCache; 
        gpTpTimeoutHandlerCache = NULL; 

        delete gpTpDummyHandlerCache; 
        gpTpDummyHandlerCache = NULL;
    }

    void * asio_handler_allocate(std::size_t size, TpTimeoutHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpTpTimeoutHandlerCache->allocate(size);
    }

    void asio_handler_deallocate(void * pointer, std::size_t size, TpTimeoutHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpTpTimeoutHandlerCache->deallocate(pointer, size);
    }

    void * asio_handler_allocate(std::size_t size, TpDummyHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpTpDummyHandlerCache->allocate(size);
    }

    void asio_handler_deallocate(void * pointer, std::size_t size, TpDummyHandler * pHandler)
    {
        RCF_UNUSED_VARIABLE(pHandler);
        return gpTpDummyHandlerCache->deallocate(pointer, size);
    }

    // ThreadPool

    void ThreadPool::setThreadName(const std::string &threadName)
    {
        Lock lock(mInitDeinitMutex);
        mThreadName = threadName;
    }

    std::string ThreadPool::getThreadName() const
    {
        Lock lock(mInitDeinitMutex);
        return mThreadName;
    }

    void ThreadPool::setMyThreadName()
    {
        std::string threadName = getThreadName();
        if (!threadName.empty())
        {
            setWin32ThreadName(threadName);
        }
    }

    void ThreadPool::onInit()
    {
        std::vector<ThreadInitFunctor> initFunctors;
        {
            Lock lock(mInitDeinitMutex);
            std::copy(
                mThreadInitFunctors.begin(), 
                mThreadInitFunctors.end(), 
                std::back_inserter(initFunctors));
        }

        for (std::size_t i=0; i<initFunctors.size(); ++i)
        {
            initFunctors[i]();
        }
    }

    void ThreadPool::onDeinit()
    {
        std::vector<ThreadDeinitFunctor> deinitFunctors;
        {
            Lock lock(mInitDeinitMutex);
            std::copy(
                mThreadDeinitFunctors.begin(), 
                mThreadDeinitFunctors.end(), 
                std::back_inserter(deinitFunctors));
        }

        for (std::size_t i=0; i<deinitFunctors.size(); ++i)
        {
            deinitFunctors[i]();
        }
    }

    void ThreadPool::addThreadInitFunctor(ThreadInitFunctor threadInitFunctor)
    {
        Lock lock(mInitDeinitMutex);
        mThreadInitFunctors.push_back(threadInitFunctor);
    }

    void ThreadPool::addThreadDeinitFunctor(ThreadDeinitFunctor threadDeinitFunctor)
    {
        Lock lock(mInitDeinitMutex);
        mThreadDeinitFunctors.push_back(threadDeinitFunctor);
    }

    AsioIoService * ThreadPool::getIoService()
    {
        return & mAsioIoServicePtr->mIoService;
    }

    void ThreadPool::enableMuxerType(MuxerType muxerType)
    {
        if (muxerType == Mt_Asio && !mAsioIoServicePtr)
        {
            mAsioIoServicePtr.reset( new AsioMuxer() );
        }
    }

    void ThreadPool::resetMuxers()
    {
        mAsioIoServicePtr.reset();
    }

    ThreadPool::ThreadPool(std::size_t fixedThreadCount) :
        mThreadName(),
        mStarted(false),
        mThreadMinCount(fixedThreadCount),
        mThreadMaxCount(fixedThreadCount),
        mReserveLastThread(false),
        mThreadIdleTimeoutMs(30*1000),
        mStopFlag(false),
        mBusyCount()
    {
    }

    ThreadPool::ThreadPool(std::size_t threadMinCount, std::size_t threadMaxCount) :
        mThreadName(),
        mStarted(false),
        mThreadMinCount(threadMinCount),
        mThreadMaxCount(threadMaxCount),
        mReserveLastThread(false),
        mThreadIdleTimeoutMs(30*1000),
        mStopFlag(false),
        mBusyCount()
    {
        RCF_ASSERT( 1 <= threadMinCount && threadMinCount <= threadMaxCount );
    }

    void ThreadPool::setThreadMinCount(std::size_t threadMinCount)
    {
        RCF_ASSERT( threadMinCount <= mThreadMaxCount );
        mThreadMinCount = threadMinCount;
    }

    std::size_t ThreadPool::getThreadMinCount() const
    {
        return mThreadMinCount;
    }

    void ThreadPool::setThreadMaxCount(std::size_t threadMaxCount)
    {
        RCF_ASSERT( threadMaxCount >= mThreadMinCount );
        mThreadMaxCount = threadMaxCount;
    }

    std::size_t ThreadPool::getThreadMaxCount() const
    {
        return mThreadMaxCount;
    }

    void ThreadPool::setThreadIdleTimeoutMs(boost::uint32_t threadIdleTimeoutMs)
    {
        mThreadIdleTimeoutMs = threadIdleTimeoutMs;
    }

    boost::uint32_t ThreadPool::getThreadIdleTimeoutMs() const
    {
        return mThreadIdleTimeoutMs;
    }

    void ThreadPool::setReserveLastThread(bool reserveLastThread)
    {
        mReserveLastThread = reserveLastThread;
    }

    bool ThreadPool::getReserveLastThread() const
    {
        return mReserveLastThread;
    }

    ThreadPool::~ThreadPool()
    {
        RCF_DTOR_BEGIN
            stop();
        RCF_DTOR_END
    }

    bool ThreadPool::launchThread(
        std::size_t howManyThreads)
    {
        Lock lock(mThreadsMutex);

        for (std::size_t i=0; i<howManyThreads; ++i)
        {
            RCF_ASSERT_LTEQ(mThreads.size() , mThreadMaxCount);

            if (mThreads.size() == mThreadMaxCount)
            {
                // We've hit the max thread limit.
                return false;
            }
            else if (mStopFlag)
            {
                return false;
            }
            else
            {
                ThreadInfoPtr threadInfoPtr( new ThreadInfo(*this));

                ThreadPtr threadPtr( new Thread(
                    boost::bind(
                        &ThreadPool::repeatTask,
                        this,
                        threadInfoPtr,
                        1000)));

                RCF_ASSERT(mThreads.find(threadInfoPtr) == mThreads.end());

                mThreads[threadInfoPtr] = threadPtr;                
            }
        }

        return true;
    }

    void ThreadPool::notifyBusy()
    {
        if (!getTlsThreadInfoPtr()->mBusy)
        {
            getTlsThreadInfoPtr()->mBusy = true;

            bool launchAnotherThread = false;

            {
                Lock lock(mThreadsMutex);
                ++mBusyCount;
                RCF_ASSERT_LTEQ(mBusyCount , mThreads.size());
                if (mBusyCount == mThreads.size())
                {
                    launchAnotherThread = true;
                }
            }

            if (!mStopFlag && launchAnotherThread)
            {
                bool launchedOk = launchThread();                    
                if (!launchedOk && mReserveLastThread && !mStopFlag)
                {
                    Exception e(_RcfError_AllThreadsBusy());
                    RCF_THROW(e);
                }
            }
        }
    }

    void ThreadPool::notifyReady()
    {
        ThreadInfoPtr threadInfoPtr = getTlsThreadInfoPtr();

        if (threadInfoPtr->mBusy)
        {
            threadInfoPtr->mBusy = false;

            Lock lock(mThreadsMutex);
            
            --mBusyCount;            
            
            RCF_ASSERT_LTEQ(mBusyCount , mThreads.size());
        }

        // Has this thread been idle? The touch timer is reset when the thread
        // does any work.
        if (threadInfoPtr->mTouchTimer.elapsed(mThreadIdleTimeoutMs))
        {
            // If we have more than our target count of threads running, and
            // if at least two of the threads are not busy, then let this thread
            // exit.

            Lock lock(mThreadsMutex);

            if (    mThreads.size() > mThreadMinCount 
                &&  mBusyCount < mThreads.size() - 1)
            {                
                threadInfoPtr->mStopFlag = true; 

                RCF_ASSERT( mThreads.find(threadInfoPtr) != mThreads.end() );
                mThreads.erase( mThreads.find(threadInfoPtr) );
            }
        }
    }

    ShouldStop::ShouldStop(ThreadInfoPtr threadInfoPtr) :
        mThreadInfoPtr(threadInfoPtr)
    {
    }

    bool ShouldStop::operator()() const
    {
        return 
                (mThreadInfoPtr.get() && mThreadInfoPtr->mStopFlag)
            ||  (mThreadInfoPtr.get() && mThreadInfoPtr->mThreadPool.shouldStop());
    }

    void ThreadPool::cycle(int timeoutMs, ShouldStop & shouldStop)
    {
        if (mAsioIoServicePtr.get() && !shouldStop())
        {
            mAsioIoServicePtr->cycle(timeoutMs);
        }

        if ( (mTask ? true : false) && !shouldStop())
        {
            mTask(timeoutMs);
        }
    }

    class ThreadLocalData;

    void ThreadPool::repeatTask(
        RCF::ThreadInfoPtr threadInfoPtr,
        int timeoutMs)
    {
        setTlsThreadInfoPtr(threadInfoPtr);

        setMyThreadName();

        onInit();

        // Put it on the stack so we can see it in the debugger.
        ThreadLocalData& tld = getThreadLocalData();
        RCF_UNUSED_VARIABLE(&tld);

        ShouldStop shouldStop(threadInfoPtr);
        while (!shouldStop())
        {
            try
            {
                while (!shouldStop())
                {
                    cycle(timeoutMs, shouldStop);
                    notifyReady();
                }
            }
            catch(const std::exception &e)
            {
                RCF_LOG_1()(e)(mThreadName) << "Thread pool: std::exception caught at top level."; 
            }
            catch(...)
            {
                RCF_LOG_1()(mThreadName) << "Thread pool: Unknown exception (...) caught at top level."; 
            }
        }

        onDeinit();

        {
            Lock lock(mThreadsMutex);
            ThreadMap::iterator iter = mThreads.find(threadInfoPtr);
            if (iter != mThreads.end())
            {
                mThreads.erase(iter);
            }            
        }

        RCF_LOG_2()(getThreadName()) << "ThreadPool - thread terminating.";

        clearThreadLocalDataForThisThread();
    }

    // not synchronized
    void ThreadPool::start()
    {
        if (!mStarted)
        {
            mStopFlag = false;

            if (mAsioIoServicePtr)
            {
                mAsioIoServicePtr->startTimer();
            }

            {
                Lock lock(mThreadsMutex);
                RCF_ASSERT(mThreads.empty())(mThreads.size());
                mThreads.clear();
                mBusyCount = 0;
            }

            bool ok = launchThread(mThreadMinCount);
            RCF_ASSERT(ok);
            RCF_UNUSED_VARIABLE(ok);

            mStarted = true;
        }
    }

    void ThreadPool::stop()
    {
        if (mStarted)
        {
            mStopFlag = true;

            ThreadMap threads;
            {
                Lock lock(mThreadsMutex);
                threads = mThreads;
            }

            ThreadMap::iterator iter;
            for (
                iter = threads.begin(); 
                iter != threads.end(); 
                ++iter)
            {
                if (mStopFunctor)
                {
                    mStopFunctor();
                }

                if (mAsioIoServicePtr)
                {
                    mAsioIoServicePtr->stopCycle();
                }

                iter->second->join();
            }

            RCF_ASSERT( mThreads.empty() );
            mThreads.clear();
            mStarted = false;
        }
    }

    bool ThreadPool::isStarted()
    {
        return mStarted;
    }

    void ThreadPool::setTask(Task task)
    {
        RCF_ASSERT(!mStarted);
        mTask = task;
    }

    void ThreadPool::setStopFunctor(StopFunctor stopFunctor)
    {
        RCF_ASSERT(!mStarted);
        mStopFunctor = stopFunctor;
    }

    std::size_t ThreadPool::getThreadCount()
    {
        Lock lock(mThreadsMutex);
        return mThreads.size();
    }

    bool ThreadPool::shouldStop() const
    {
        return mStopFlag;
    }

    ThreadTouchGuard::ThreadTouchGuard() : 
        mThreadInfoPtr(getTlsThreadInfoPtr())
    {
        if (mThreadInfoPtr)
        {
            mThreadInfoPtr->touch();
        }
    }

    ThreadTouchGuard::~ThreadTouchGuard()
    {
        if (mThreadInfoPtr)
        {
            mThreadInfoPtr->touch();
        }
    }

    ThreadInfo::ThreadInfo(ThreadPool & threadPool) :
        mThreadPool(threadPool),
        mBusy(),
        mStopFlag()
    {}

    void ThreadInfo::touch()
    {
        mTouchTimer.restart();
    }

    void ThreadInfo::notifyBusy()
    {
        touch();
        mThreadPool.notifyBusy();
    }

    ThreadPool & ThreadInfo::getThreadPool()
    {
        return mThreadPool;
    }

} // namespace RCF
