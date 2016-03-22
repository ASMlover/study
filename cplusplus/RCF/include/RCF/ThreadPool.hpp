
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

#ifndef INCLUDE_RCF_THREADMANAGER_HPP
#define INCLUDE_RCF_THREADMANAGER_HPP

#include <vector>

#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/AsioFwd.hpp>
#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Timer.hpp>
#include <RCF/Tools.hpp>

namespace boost {
    namespace asio {
        class io_service;
    }
}

namespace RCF {

    class                                               RcfServer;
    typedef boost::function1<void, int>                 Task;
    class                                               TaskEntry;
    typedef boost::function0<void>                      StopFunctor;

    class                                               ThreadPool;
    typedef boost::shared_ptr<ThreadPool>               ThreadPoolPtr;

    class                                               AsioMuxer;

    typedef boost::shared_ptr<ThreadPool>               ThreadPoolPtr;
    class                                               ShouldStop;

    class RCF_EXPORT ThreadInfo
    {
    public:
        ThreadInfo(ThreadPool & threadPool);
        void touch();
        void notifyBusy();
        ThreadPool & getThreadPool();

    private:
        friend class ThreadPool;
        friend class ShouldStop;

        ThreadPool &    mThreadPool;
        bool            mBusy;
        bool            mStopFlag;
        RCF::Timer      mTouchTimer;
    };

    typedef boost::shared_ptr<ThreadInfo> ThreadInfoPtr;

    enum MuxerType
    {
        Mt_None,
        Mt_Asio
    };

    static const MuxerType DefaultMuxerType = Mt_Asio;

    /// Represents a server-side thread pool.
    class RCF_EXPORT ThreadPool : 
        public boost::enable_shared_from_this<ThreadPool>
    {
    public:

        typedef boost::function0<void> ThreadInitFunctor;
        typedef boost::function0<void> ThreadDeinitFunctor;

        // *** SWIG BEGIN ***

        ThreadPool(std::size_t fixedThreadCount);
        ThreadPool(std::size_t threadMinCount, std::size_t threadMaxCount);

        /// Sets the minimum number of threads in the thread pool.
        void            setThreadMinCount(std::size_t threadMinCount);

        /// Returns the minimum number of threads in the thread pool.
        std::size_t     getThreadMinCount() const;

        /// Sets the maximum number of threads in the thread pool.
        void            setThreadMaxCount(std::size_t threadMaxCount);

        /// Returns the maximum number of threads in the thread pool.
        std::size_t     getThreadMaxCount() const;

        /// Sets the thread idle timeout value, in milliseconds. After a thread has
        /// been idle for this time, it will be shut down, unless the thread count
        /// is already at the minimum value for the thread pool.
        void            setThreadIdleTimeoutMs(boost::uint32_t threadIdleTimeoutMs);

        /// Returns the thread idle timeout value, in milliseconds.
        boost::uint32_t getThreadIdleTimeoutMs() const;

        /// If this setting is true, clients will receive an error message right
        /// away, if all threads in the thread pool are busy. Otherwise, the client
        /// will wait for a thread in the thread pool to become free.
        void            setReserveLastThread(bool reserveLastThread);
        bool            getReserveLastThread() const;

        /// Sets the thread name of the thread pool threads, as visible in a debugger.
        void            setThreadName(const std::string & threadName);

        /// Returns the thread name of the thread pool threads.
        std::string     getThreadName() const;


        // *** SWIG END ***

        ~ThreadPool();
        
        void            start();
        void            stop();
        bool            isStarted();

        void            addThreadInitFunctor(
                            ThreadInitFunctor threadInitFunctor);

        void            addThreadDeinitFunctor(
                            ThreadDeinitFunctor threadDeinitFunctor);

        AsioIoService * getIoService();

        void            notifyBusy();

        std::size_t     getThreadCount();

        void            setTask(Task task);
        void            setStopFunctor(StopFunctor stopFunctor);

        void            enableMuxerType(MuxerType muxerType);
        void            resetMuxers();


        bool            shouldStop() const;

    private:

        void            onInit();
        void            onDeinit();
        void            setMyThreadName();

        bool            launchThread(std::size_t howManyThreads = 1);

        void            notifyReady();

        void            repeatTask(
                            RCF::ThreadInfoPtr threadInfoPtr,
                            int timeoutMs);

        void            cycle(int timeoutMs, ShouldStop & shouldStop);

        friend class                        TaskEntry;
        friend class                        RcfServer;

        mutable Mutex                       mInitDeinitMutex;
        std::vector<ThreadInitFunctor>      mThreadInitFunctors;
        std::vector<ThreadDeinitFunctor>    mThreadDeinitFunctors;
        std::string                         mThreadName;
        boost::shared_ptr<AsioMuxer>        mAsioIoServicePtr;

        bool                                mStarted;
        std::size_t                         mThreadMinCount;
        std::size_t                         mThreadMaxCount;
        bool                                mReserveLastThread;
        boost::uint32_t                     mThreadIdleTimeoutMs;

        Task                                mTask;
        StopFunctor                         mStopFunctor;

        bool                                mStopFlag;

        typedef std::map<ThreadInfoPtr, ThreadPtr> ThreadMap;

        Mutex                               mThreadsMutex;
        ThreadMap                           mThreads;
        std::size_t                         mBusyCount;
    };    

    class ThreadTouchGuard
    {
    public:
        ThreadTouchGuard();
        ~ThreadTouchGuard();
    private:
        ThreadInfoPtr mThreadInfoPtr;
    };

    class ShouldStop
    {
    public:

        ShouldStop(
            ThreadInfoPtr threadInfoPtr);

        bool operator()() const;

    private:
        friend class ThreadPool;

        ThreadInfoPtr mThreadInfoPtr;
    };

    RCF_EXPORT void setWin32ThreadName(const std::string & threadName);

} // namespace RCF

#endif // ! INCLUDE_RCF_THREADMANAGER_HPP
